# 基本动态压缩器开发指南

[具体代码请参考此链接](..\plugins\dynamics\base_compressor.h)

---

## 一、概述

动态范围压缩器（Dynamic Range Compressor）是一种自动增益控制效果器。它通过**降低高电平信号的增益、保留低电平信号**，来缩小音频信号的动态范围，使整体音量更加稳定、饱满。

本设计实现了一个**完整的软膝（Soft Knee）压缩器**，包含以下核心模块：

| 模块 | 功能 |
|------|------|
| 电平检测（Level Detection） | 将输入采样值转换为 dB 表示 |
| 增益计算（Gain Computer） | 根据阈值、压缩比和软膝计算目标压缩量 |
| 包络跟随器（Envelope Follower） | 一阶低通滤波器模拟 Attack / Release 时间响应 |
| 补偿增益（Makeup Gain） | 在输出端提升整体电平以弥补压缩造成的音量损失 |

---

## 二、参数定义

### 2.1 Open / Close（开关）
- **类型**：Toggle Button / 0-1 浮点参数
- **说明**：控制压缩器是否生效。关闭时信号直通（Bypass）。

### 2.2 Threshold（阈值）
- **单位**：dBFS
- **范围**：-60.0 dB ~ 0.0 dB
- **默认值**：-20.0 dB
- **说明**：信号电平超过该值时开始压缩。阈值越低，被压缩的信号范围越大。

### 2.3 Ratio（压缩比）
- **范围**：1.0 : 1 ~ 20.0 : 1
- **默认值**：4.0 : 1
- **说明**：表示输入电平超出阈值部分与输出电平增量的比例。例如 Ratio = 4:1 表示输入超出阈值 4 dB 时，输出仅增加 1 dB。

### 2.4 Attack Time（启动时间）
- **单位**：ms（毫秒）
- **范围**：0.1 ms ~ 200.0 ms
- **默认值**：10.0 ms
- **说明**：当信号超过阈值时，压缩器开始工作的速度（增益衰减的快慢）。较短的 Attack 能捕捉瞬态；较长的 Attack 保留音头的冲击感。

### 2.5 Release Time（释放时间）
- **单位**：ms（毫秒）
- **范围**：10.0 ms ~ 2000.0 ms
- **默认值**：100.0 ms
- **说明**：当信号回落到阈值以下时，压缩器恢复正常增益的速度。较短的 Release 响应快速变化；较长的 Release 能避免"抽吸效应"（pumping）。

### 2.6 Makeup Gain（补偿增益）
- **单位**：dB
- **范围**：0.0 dB ~ 24.0 dB
- **默认值**：0.0 dB
- **说明**：在压缩处理后对信号施加的固定增益提升，用于补偿压缩造成的电平损失。

---

## 三、信号处理流程

```
输入信号
    │
    ▼
┌─────────────────────────────────┐
│ 1. 参数同步与平滑                │
│    syncParametersFromAPVTS()     │
│    updateProcessorParameters()   │
│    (SmoothedValue 线性插值)       │
└─────────────────────────────────┘
    │
    ▼
┌─────────────────────────────────┐
│ 2. 电平检测 (Level Detection)    │
│    inputDB = 20 * log10(|x|)     │
└─────────────────────────────────┘
    │
    ▼
┌─────────────────────────────────┐
│ 3. 增益计算 (Gain Computer)      │
│    - 计算 over = inputDB - T     │
│    - 三段软膝判断                 │
│    - 输出 gainReductionDB        │
└─────────────────────────────────┘
    │
    ▼
┌─────────────────────────────────┐
│ 4. 包络跟随 (Attack / Release)  │
│    - 一阶 IIR 低通滤波器          │
│    - Attack α 或 Release α       │
│    - 当前 gainReductionDB 平滑   │
└─────────────────────────────────┘
    │
    ▼
┌─────────────────────────────────┐
│ 5. 补偿增益 (Makeup Gain)        │
│    finalGainDB = gainDB + M      │
│    finalGain = 10^(finalGainDB/20)│
└─────────────────────────────────┘
    │
    ▼
┌─────────────────────────────────┐
│ 6. 输出信号                     │
│    y[n] = x[n] * finalGain      │
└─────────────────────────────────┘
```

---

## 四、各模块详细设计

### 4.1 参数平滑（Parameter Smoothing）

使用 `juce::SmoothedValue<float, Linear>` 对每个参数进行线性插值平滑，防止参数突变导致的音质爆音（zipper noise）。

- 在 `prepareToPlay()` 中调用 `.reset(sampleRate, 0.02)` 设置平滑时间为 20 ms。
- 在 `updateProcessorParameters()` 中调用 `.setTargetValue(value)` 设置目标值。
- 在采样点循环中调用 `.getNextValue()` 逐采样点获取当前插值。

### 4.2 电平检测（Level Detection）

将输入采样值的**绝对值**转换为 dB 刻度：

```
inputDB = gainToDecibels(|x[n]| + ε)
```

其中 ε = 1e-6 用于防止信号为静音时 log(0) 导致负无穷大。

### 4.3 增益计算机（Gain Computer）—— 软膝压缩曲线

本设计使用三段式软膝曲线，膝范围（knee width）固定为 `kneeRangeDB = 10.0 dB`。

#### 数学推导

定义：
- `slope = 1 - 1 / ratio`：压缩斜率（0 表示无压缩，1 表示无限压缩）
- `over = inputDB - thresholdDB`：电平超过阈值的量（dB）
- `W = kneeRangeDB`：软膝范围宽度

**三段判断：**

| 区域 | 条件 | 增益衰减量 gainDB |
|------|------|------------------|
| 不压缩区 | `over ≤ -W/2` | `0`（不压缩） |
| 软膝过渡区 | `-W/2 < over < W/2` | `-slope × (over + W/2)² / (2W)` |
| 线性压缩区 | `over ≥ W/2` | `-over × slope` |

#### 软膝公式推导

为了让软膝区在两端与相邻区域**连续且一阶导数连续**（C¹ 连续），我们设计一条抛物线：

在 `over = -W/2` 处：
- 值 = 0，斜率 = 0（与不压缩区衔接）

在 `over = W/2` 处：
- 值 = `-slope × W / 2`（与线性压缩区衔接，当 over = W/2 时，线性区值为 `-W/2 × slope`）
- 斜率 = `-slope`（与线性压缩区导数一致）

构造中间变量 `k = over + W/2`，当 `over = -W/2` 时 `k = 0`；当 `over = W/2` 时 `k = W`。

则抛物线方程为：
```
gainDB = -slope × k² / (2W)
```

验证：
- 在 `k = 0`（over = -W/2）：gainDB = 0 ✅
- 在 `k = W`（over = W/2）：gainDB = `-slope × W² / (2W) = -slope × W/2` ✅
- 导数 `dgainDB/dk = -slope × k / W`，在 k = 0 处为 0 ✅，在 k = W 处为 `-slope` ✅

### 4.4 Attack / Release 包络跟随器（Envelope Follower）

包络跟随器本质上是一个**一阶低通滤波器**（单极点平滑器），用于平滑增益计算机输出的增益衰减量。Attack Time 和 Release Time 分别控制滤波器在信号电平上升和回落时的响应速度。

以下从连续时间系统出发，经冲激不变法和零阶保持法推导出差分方程。

---

#### 4.4.1 一阶低通滤波器的微分方程

连续时间一阶低通滤波器的输入输出关系可由如下微分方程描述：

$$
\tau \frac{dy(t)}{dt} + y(t) = x(t)
$$

其中 $\tau$ 为时间常数（单位：秒），$x(t)$ 为输入信号，$y(t)$ 为输出信号。

对两边做 Laplace 变换（零初始条件），得到系统的传递函数：

$$
H(s) = \frac{Y(s)}{X(s)} = \frac{1}{\tau s + 1} = \frac{1/\tau}{s + 1/\tau}
$$

该系统的单位冲激响应为：

$$
h(t) = \mathcal{L}^{-1}\{H(s)\} = \frac{1}{\tau} e^{-t/\tau} \, u(t)
$$

其中 $u(t)$ 为单位阶跃函数。系统的极点位于 $s = -1/\tau$，对应特征频率 $f_c = 1/(2\pi\tau)$，即 $-3\text{ dB}$ 截止频率。

阶跃响应为：

$$
s(t) = \int_{0}^{t} h(\lambda) d\lambda = \left(1 - e^{-t/\tau}\right) u(t)
$$

当 $t = \tau$ 时，$s(\tau) = 1 - e^{-1} \approx 0.632$，即输出上升到最终值的 **63.2%**，这正是时间常数 $\tau$ 的物理含义。

---

#### 4.4.2 冲激不变法（Impulse Invariance）

冲激不变法的核心思想是：让离散时间系统的单位冲激响应等于连续时间系统冲激响应的采样：

$$
h[n] = T_s \cdot h(nT_s) = \frac{T_s}{\tau} \, e^{-nT_s/\tau} \, u[n]
$$

其中 $T_s = 1/f_s$ 为采样周期。对 $h[n]$ 做 Z 变换：

$$
H(z) = \sum_{n=0}^{\infty} h[n] z^{-n} = \frac{T_s}{\tau} \cdot \frac{1}{1 - e^{-T_s/\tau} z^{-1}}
$$

由此得到差分方程：

$$
y[n] = e^{-T_s/\tau} \, y[n-1] + \frac{T_s}{\tau} \, x[n]
$$

**问题**：该方法的直流增益为：

$$
H(1) = \frac{T_s/\tau}{1 - e^{-T_s/\tau}} \neq 1
$$

即直流增益不为 1（只有当 $T_s \ll \tau$ 时才近似为 1），这意味着冲激不变法在低频段会引入增益误差。对于压缩器的包络跟随器，直流增益必须精确为 1（否则压缩量会产生静差），因此需要采用能保持直流增益的方法。

---

#### 4.4.3 零阶保持法（Zero-Order Hold, ZOH）

ZOH 法（也称**阶跃不变法**，Step-Invariant Method）假设 DAC 在采样间隔内将信号保持为常数：

$$
x(t) = x[n], \quad t \in [nT_s, \, (n+1)T_s)
$$

该方法通过匹配连续时间系统的**阶跃响应**在采样点上的值来获得离散等效。

ZOH 离散化的公式为：

$$
H(z) = (1 - z^{-1}) \, \mathcal{Z}\left\{ \mathcal{L}^{-1}\left[ \frac{H(s)}{s} \right] \right\}
$$

代入 $H(s) = \dfrac{1/\tau}{s + 1/\tau}$：

$$
\frac{H(s)}{s} = \frac{1/\tau}{s(s + 1/\tau)} = \frac{1}{s} - \frac{1}{s + 1/\tau}
$$

$$
\mathcal{L}^{-1}\left\{ \frac{H(s)}{s} \right\} = \left(1 - e^{-t/\tau}\right) u(t)
$$

采样得：

$$
g[n] = \left(1 - e^{-nT_s/\tau}\right) u[n]
$$

其 Z 变换为：

$$
G(z) = \frac{1}{1 - z^{-1}} - \frac{1}{1 - e^{-T_s/\tau} z^{-1}}
       = \frac{(1 - e^{-T_s/\tau}) z^{-1}}{(1 - z^{-1})(1 - e^{-T_s/\tau} z^{-1})}
$$

因此：

$$
H(z) = (1 - z^{-1}) G(z) = \frac{1 - e^{-T_s/\tau}}{1 - e^{-T_s/\tau} z^{-1}} \cdot z^{-1}
$$

此即 ZOH 等效的传递函数，对应差分方程：

$$
y[n] = e^{-T_s/\tau} \, y[n-1] + (1 - e^{-T_s/\tau}) \, x[n-1]
$$

直流增益验证：

$$
H(1) = \frac{1 - e^{-T_s/\tau}}{1 - e^{-T_s/\tau}} \cdot 1 = 1 \quad \checkmark
$$

直流增益精确为 1，满足压缩器包络跟随的要求。

---

#### 4.4.4 一阶低通滤波器的差分方程

ZOH 方法给出的差分方程使用了 $x[n-1]$（上一时刻的输入）。在代码实现中，为了减少一采样周期的相位延迟，通常将输入向前对齐一个采样点，即使用当前输入 $x[n]$：

$$
y[n] = e^{-T_s/\tau} \, y[n-1] + (1 - e^{-T_s/\tau}) \, x[n]
$$

此变换仅相当于在传递函数中乘以 $z$（一采样点的时间提前），不影响幅频响应：

$$
H_{\text{code}}(z) = z \cdot H_{\text{ZOH}}(z) = \frac{1 - e^{-T_s/\tau}}{1 - e^{-T_s/\tau} z^{-1}}
$$

令：

$$
\alpha = 1 - e^{-T_s/\tau}, \quad 1 - \alpha = e^{-T_s/\tau}
$$

得到代码中使用的标准一阶 IIR 低通滤波器形式：

$$
\boxed{\, y[n] = \alpha \, x[n] + (1 - \alpha) \, y[n-1] \,}
$$

其中系数 $\alpha$ 由时间常数 $\tau$ 和采样率 $f_s = 1/T_s$ 共同决定：

$$
\boxed{\, \alpha = 1 - \exp\left(-\frac{1}{f_s \cdot \tau}\right) \,}
$$

对应代码中的实现：

```cpp
attackAlpha = 1 - std::exp(-1 / (sampleRate * attackTimeMs * 0.001f));
```

当 $T_s \ll \tau$ 时，做一阶 Taylor 展开可得近似：

$$
\alpha = 1 - e^{-T_s/\tau} \approx \frac{T_s}{\tau} \quad (T_s \ll \tau)
$$

代码中使用精确的指数形式而非线性近似，保证了在**低采样率**或**短时间常数**（如 Attack = 0.1 ms）等极端条件下滤波器的稳定性与准确性。

---

#### 4.4.5 Attack / Release 切换逻辑

Attack 和 Release 使用相同的滤波器结构，但采用不同的时间常数：

$$
\alpha_{\text{attack}} = 1 - e^{-T_s/\tau_{\text{attack}}}, \quad
\alpha_{\text{release}} = 1 - e^{-T_s/\tau_{\text{release}}}
$$

切换逻辑通过比较**当前理论增益衰减量**与**上一个已平滑的增益衰减量**来决定使用哪个 $\alpha$：

```
if gainComputerOutput[n] < y[n-1] ：（增益在减小，即信号电平在上升）
    α = attackAlpha   （加速响应，快速追踪瞬态）
else：（增益在恢复，即信号电平在回落）
    α = releaseAlpha  （减速响应，控制恢复速度）
```

这种设计使压缩器在信号骤起时快速作用（Attack），在信号回落后缓慢恢复（Release），符合压缩器的时间特性需求。

#### 4.4.6 左右声道共享 Alpha 的说明

左声道和右声道各有一个 `attackAndReleaseFilter` 实例，各自维护独立的 $y[n-1]$ 状态。但由于 $\alpha_{\text{attack}}$ 和 $\alpha_{\text{release}}$ 仅由参数值和采样率决定，与声道信号无关，因此右声道直接复用左声道的 Alpha 值：

```cpp
attackAndReleaseRight.setValue(attackAndReleaseLeft.attackAlpha);
```

两个实例的区别在于各自维护的 $y_1$（上一个平滑输出），因为左右声道的音频信号不同，包络状态也不同，但滤波器系数相同。

### 4.5 补偿增益（Makeup Gain）

在处理链最后，将平滑后的增益衰减量加上补偿增益：

```
finalGainDB = gainDB + makeupGainDB
finalLinearGain = 10^(finalGainDB / 20)
outputSample = inputSample × finalLinearGain
```

补偿增益在 dB 域相加，然后一并转换为线性增益应用到信号上。

---

## 五、完整处理伪代码

```
for each sample index:
    // 1. 获取当前平滑参数
    T    = smoothedThreshold.getNextValue()
    R    = smoothedRatio.getNextValue()
    att  = smoothedAttackTime.getNextValue()
    rel  = smoothedReleaseTime.getNextValue()
    M    = smoothedMakeupGain.getNextValue()

    // 2. 若参数正在平滑中，更新滤波器 Alpha
    if smoothedAttackTime.isSmoothing():
        setAttackAlpha(sampleRate, att)
    if smoothedReleaseTime.isSmoothing():
        setReleaseAlpha(sampleRate, rel)

    // 3. 电平检测
    inputDB = 20 * log10(|inputSample| + ε)

    // 4. 增益计算（软膝）
    over = inputDB - T
    slope = 1 - 1 / R
    if over >= W/2:
        gainDB = -over * slope
    else if over > -W/2:
        k = over + W/2
        gainDB = -slope * k² / (2W)
    else:
        gainDB = 0

    // 5. Attack / Release 包络
    if gainDB < y1:
        setValue(attackAlpha)
    else:
        setValue(releaseAlpha)
    gainDB = onePoleFilter(gainDB)  // y[n] = α * gainDB + (1-α) * y1

    // 6. 补偿增益 + 输出
    finalGain = 10^((gainDB + M) / 20)
    outputSample = inputSample * finalGain
```

---

## 六、常见调试问题

### 6.1 声音失真
- Release Time 过短可能导致快速恢复产生可听失真
- 补偿增益过大可能导致削波

### 6.2 抽吸效应（Pumping）
- Release Time 设置过短，增益恢复过快，在背景音上产生可听的"呼吸感"
- 可适当增大 Release Time 缓解

### 6.3 音头被过度压缩
- Attack Time 设置过短，瞬态信号在到达峰值前就被压缩
- 适当增大 Attack Time 让音头通过后再压缩

---

## 七、扩展思路

1. **侧链滤波（Sidechain）**：将增益计算机的输入替换为经过滤波的信号（如低频信号），实现闪避效果器（Ducker）
2. **RMS 电平检测**：用 RMS 值替代瞬时峰值作为电平检测输入，得到更平滑的压缩行为
3. **前视压缩（Lookahead）**：在信号路径中加入延迟（~1-10 ms），使增益计算"预见"即将到来的信号，减少失真
4. **多种膝形状**：可调膝宽（Knee Width）或各种膝曲线形态