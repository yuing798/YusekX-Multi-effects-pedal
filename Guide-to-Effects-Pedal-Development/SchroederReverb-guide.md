# 施罗德混响效果器开发指南

[具体代码请参考此链接](..\plugins\Reverb\SchroederReverb.h)


## 概述

本文档介绍基于 Schroeder 混响模型的数字混响效果器的 DSP 实现原理。Schroeder 混响是数字混响领域的奠基性算法，由 Manfred Schroeder 于 1960 年代提出，其核心思路是用**并联梳状滤波器**模拟早期反射的密度与能量衰减，再用**串联全通滤波器**对回声进行时间上的扩散，从而在计算资源有限的条件下合成出令人信服的空间感。

完整信号链如下：

```
输入 ──► 预延迟 ──► 并联梳状滤波器
                    （内置低通滤波器）
                          │
                          ▼
                    串联全通滤波器
                          │
                          ▼
                    干湿混合 ──► 增益补偿 ──► 输出
```

---

## 一、预延迟（Pre-Delay）

### 原理

在真实的声学空间中，声源发出的声音到达听者之前，会先经过一段自由传播时间才出现第一个反射波。这段时间差（通常为几毫秒至数十毫秒）称为**预延迟**，它强烈影响听者对空间大小的感知——预延迟越长，感觉房间越大。

预延迟本质上是一段固定长度的**环形缓冲区（Circular Buffer）**，每次写入一个新样本，同时从 $D$ 个样本之前的位置读出：

$$y[n] = x[n - D]$$

其中 $D$ 由延迟时间（毫秒）换算到样本数：

$$D = \left\lfloor t_{\text{ms}} \times \frac{f_s}{1000} \right\rfloor$$

### 实现要点

为了支持非整数的延迟样本数（例如用户拖动参数时平滑过渡），读指针位置采用**线性插值**在两个相邻样本之间估算：

```cpp
float processSample(float inputSample) {
    float readIndex = getCircularBufferIndex(
        writeIndex - delaySamplesNum, preDelayBuffer.size());
    float readSample = getLinearInterpolator(
        preDelayBuffer.data(),
        static_cast<int>(preDelayBuffer.size()),
        readIndex);
    writeIndex = getCircularBufferIndex(writeIndex + 1, preDelayBuffer.size());
    preDelayBuffer[writeIndex] = inputSample;
    return readSample;
}
```

左右声道各维护一个独立的预延迟实例（`preDelayL` / `preDelayR`），保证立体声处理的独立性。

---

## 二、并联梳状滤波器（Parallel Comb Filters）

### 梳状滤波器的基本结构

反馈梳状滤波器（Feedback Comb Filter）是混响中最核心的构件，其差分方程为：

$$y[n] = x[n] + g \cdot y[n - D]$$

其中 $g$ 为反馈系数（`decayLevel`），$D$ 为延迟样本数。从频域来看，这个结构在频率响应上呈现出梳齿状的峰谷（因此得名），同时由于反馈回路的存在，每经过一次循环能量以 $g$ 的比例衰减，形成**指数衰减的回声序列**，这正是混响尾音的物理基础。

> **梳状滤波器的作用**
>
> 梳状滤波器的核心贡献有两个：
>
> 1. **产生离散回声**：单个梳状滤波器会生成一系列等间隔的回声（间隔为 $D$ 个样本），其能量随时间按 $g^k$（$k$ 为回声次数）指数衰减，直接模拟声波在两面平行墙壁之间来回反射的物理过程。
> 2. **控制混响时间（RT60）**：通过调节反馈系数 $g$，可以精确控制混响能量衰减 60 dB 所需的时间。$g$ 越接近 1，混响尾音越长。

### 为什么要并联？

> **为什么梳状滤波器要并联**
>
> 单个梳状滤波器的回声之间间隔均匀、规律性强，在听感上会产生明显的"金属音色"（metallic coloration），而不像真实空间中那样密集而均匀的反射包络。将多个延迟时间**互质**的梳状滤波器**并联**，各自独立地对输入信号产生不同周期的回声序列，然后将输出**叠加**：
>
> $$y[n] = \frac{1}{N} \sum_{k=1}^{N} y_k[n]$$
>
> 由于不同延迟周期的回声在时间上交错分布，叠加后的回声密度大幅提升，频率响应趋于平坦，"金属感"显著降低，听感更接近真实的扩散混响场。本实现使用 **8 个**梳状滤波器并联，并将输出平均，以避免叠加带来的增益放大。

### 实现细节

8 个梳状滤波器的基础延迟时间来自一张预设的查找表（`combDelayLineLookUp`），在初始化时根据采样率和房间尺寸比例缩放，并取最近的素数：

```cpp
delaySamplesNum = getNearestPrimeNumber(
    combDelayLineValue * sampleRate / defaultSampleRate * roomSize);
```

> **为什么延迟时间要取素数？**
>
> 若两个梳状滤波器的延迟时间存在公约数，则它们的回声序列会在某些时刻同时到达，造成局部能量集中，频率响应出现明显的峰值（即"共振"），破坏混响的扩散均匀性。将延迟时间取为**互质的素数**，可以将任意两个梳状滤波器的最小公倍数最大化，使得回声序列在时域上尽可能均匀交错，不存在共同的周期，从而得到最为平坦、均匀的能量包络。这是 Schroeder 算法中一个经典的工程技巧。

并联处理与写指针推进：

```cpp
float processSample(float inputSample, float decay) {
    float outputSample = 0.0f;
    outputSample += comb1.processSample(inputSample, decay);
    // ...（comb2 ~ comb8 同理）
    comb1.writeIndex = getCircularBufferIndex(comb1.writeIndex + 1, combBufferSize);
    // ...（其余写指针同步推进）
    return outputSample / 8.0f;
}
```

---

## 三、低通滤波器（Damp Filter，内置于梳状滤波器）

### 为什么低通滤波器放在梳状滤波器的反馈回路内？

> **低通滤波器的位置与作用**
>
> 在真实的声学环境中，高频声波比低频声波更容易被空气和墙壁材料吸收，因此混响的高频成分衰减速度远快于低频成分——远处的回声听起来总是"更暗"、更浑浊。
>
> 将低通滤波器放置在梳状滤波器的**反馈回路内部**（即对反馈信号先滤波再写入缓冲区），可以精确模拟这一物理过程：每经过一次反馈循环，高频能量就被衰减一次，循环次数越多（即混响时间越长），高频损失越大，与真实房间中高频衰减先于低频的规律完全吻合。若将低通滤波器放在梳状滤波器之外（全通滤波器之前），则所有频率成分的混响时间相同，缺乏真实感。

### 一阶低通滤波器结构

本实现采用最简单的一阶 IIR 低通滤波器：

$$y[n] = b_0 \cdot x[n] - a_1 \cdot y[n-1]$$

其中系数由阻尼参数 `dampLevel` 决定：

$$b_0 = 1 - d, \quad a_1 = -d$$

`dampLevel` 越大，高频截止越低，混响"越暗"。

```cpp
float processSample(float inputSample) {
    float outputSample = b0 * inputSample - a1 * y1;
    y1 = outputSample;
    return outputSample;
}
```

每个梳状滤波器内置独立的 `lowPassFilter` 实例，确保各并联支路的高频衰减独立进行。在梳状滤波器的单样本处理中，低通滤波器作用于**从延迟线读回的样本**，然后再乘以反馈系数回写：

```cpp
float processSample(float inputSample, float decay) {
    float readSample = /* 从环形缓冲区读取 */;
    readSample = dampFilter.processSample(readSample); // 先低通
    combDelayLineBuffer[writeIndex] = inputSample + decay * readSample; // 再反馈
    return combDelayLineBuffer[writeIndex];
}
```

---

## 四、串联全通滤波器（Series All-Pass Filters）

### 全通滤波器的基本结构

全通滤波器（All-Pass Filter）的特点是**幅度响应在所有频率上均为常数**（增益恒为 1），但**相位响应随频率变化**。其差分方程为：

$$y[n] = -g \cdot x[n] + x[n - D] + g \cdot y[n - D]$$

其中 $g$ 为扩散系数（`diffusionLevel`），$D$ 为延迟样本数。

> **全通滤波器的作用**
>
> 全通滤波器在混响中的职责是**时间扩散（temporal diffusion）**，而非频率选择。经过并联梳状滤波器之后，信号的能量包络虽然已经足够密集，但回声在时间轴上仍然存在一定的规律性。全通滤波器通过引入频率相关的相位延迟，将这些回声在时间上进一步"打散"，使不同频率成分的到达时间产生细微的随机错位，从而获得更加均匀、自然的混响扩散感（即更高的"回声密度"）。由于全通滤波器不改变任何频率的幅度，它可以纯粹地改善混响的时域质感，而不影响整体的频率平衡。

### 为什么要串联？

> **为什么全通滤波器要串联**
>
> 单级全通滤波器能提供的相位扩散量有限，不足以将梳状滤波器输出中残余的周期性完全消除。将多个全通滤波器**串联**，每一级的输出作为下一级的输入，相位扩散效果**逐级叠加**：
>
> $$y = \text{AP}_4(\text{AP}_3(\text{AP}_2(\text{AP}_1(x))))$$
>
> 这样，每个频率成分所经历的总相位延迟是各级相位响应之和，最终在时域上产生更加丰富、平滑的扩散效果。串联而非并联的原因在于：并联会使各级信号在时域叠加，可能重新引入规律性的回声峰；而串联则是将扩散效果顺序"堆叠"，持续增加时间上的复杂度。本实现使用 **4 级**串联全通滤波器，其延迟时间同样取互质的素数（原因与梳状滤波器相同），进一步避免周期性。

### 实现细节

```cpp
float processSample(float inputSample, float diffusion) {
    int readIndex = getCircularBufferIndex(
        writeIndex - delaySamplesNum,
        static_cast<int>(allPassDelayLineBuffer.size()));
    float readSample = allPassDelayLineBuffer[readIndex];
    float outputSample = -diffusion * inputSample + readSample;
    allPassDelayLineBuffer[writeIndex] = inputSample + diffusion * readSample;
    return outputSample;
}
```

4 级串联处理：

```cpp
float processSample(float inputSample, float diffusion) {
    float outputSample = inputSample;
    outputSample = allPass1.processSample(outputSample, diffusion);
    outputSample = allPass2.processSample(outputSample, diffusion);
    outputSample = allPass3.processSample(outputSample, diffusion);
    outputSample = allPass4.processSample(outputSample, diffusion);
    // 写指针同步推进
    return outputSample;
}
```

---

## 五、干湿混合（Dry/Wet Mix）

干湿混合将原始干信号与混响湿信号按比例叠加，控制混响的深度感：

$$y[n] = w_{\text{wet}} \cdot y_{\text{reverb}}[n] + w_{\text{dry}} \cdot x_{\text{dry}}[n]$$

为避免在实时处理循环中反复调用 `powf` 等耗时函数，干湿增益系数预先计算并存储在两张查找表（`dryTable` / `wetTable`）中，处理时通过 `mixLevel` 参数索引直接读取：

```cpp
float currentDry = dryTable[static_cast<int>(currentMixLevel * (dryTable.size() - 1))];
float currentWet = wetTable[static_cast<int>(currentMixLevel * (wetTable.size() - 1))];
```

典型的设计是令 $w_{\text{dry}}^2 + w_{\text{wet}}^2 = 1$（等功率混合），保证在任意混合比例下总输出功率恒定，避免出现在中间位置时感知音量下降的问题。

在 `processBlock` 中，干信号在进入混响链之前被单独保存，最后再与湿信号混合：

```cpp
float drySampleLeft = inputSampleLeft;           // 保存干信号
inputSampleLeft = preDelayL.processSample(inputSampleLeft);
inputSampleLeft = combFiltersL.processSample(inputSampleLeft, currentDecayLevel);
inputSampleLeft = allPassFiltersL.processSample(inputSampleLeft, currentDiffusionLevel);
// 干湿混合
channelDataLeft[sampleIndex] =
    (inputSampleLeft * currentWet + drySampleLeft * currentDry) * makeUpGain;
```

---

## 六、增益补偿（Make-Up Gain）

混响效果器在加入大量湿信号后，整体感知响度会发生变化。增益补偿提供一个可调的线性增益，以 dB 为单位输入，转换为线性乘数后作用于最终输出：

$$G_{\text{linear}} = 10^{\,G_{\text{dB}}\,/\,20}$$

最终输出为：

$$y_{\text{out}}[n] = \left( w_{\text{wet}} \cdot y_{\text{reverb}}[n] + w_{\text{dry}} \cdot x_{\text{dry}}[n] \right) \times G_{\text{linear}}$$

---

## 完整信号流汇总

```
x[n]
  │
  ├──────────────────────────────────────── 干信号保存
  │
  ▼
预延迟（环形缓冲区 + 线性插值）
  │  y[n] = x[n - D_pre]
  ▼
┌──────────────────────────────────────────────────────┐
│  并联梳状滤波器 × 8（内置低通滤波器）                │
│                                                      │
│  每个支路：                                          │
│    y_k[n] = x[n] + g · LPF(y_k[n - D_k])           │
│                                                      │
│  输出叠加平均：                                      │
│    y[n] = (1/8) Σ y_k[n]                            │
└──────────────────────────────────────────────────────┘
  │
  ▼
串联全通滤波器 × 4
  │  y[n] = -g·x[n] + x[n - D] + g·y[n - D]（逐级级联）
  ▼
干湿混合
  │  out[n] = w_wet · y_wet[n] + w_dry · x_dry[n]
  ▼
增益补偿
  │  out[n] = out[n] × 10^(G_dB / 20)
  ▼
y_out[n]
```

---

## 参考

- Schroeder, M. R. (1962). *Natural Sounding Artificial Reverberation*. Journal of the Audio Engineering Society.
- Moorer, J. A. (1979). *About This Reverberation Business*. Computer Music Journal.
- Zölzer, U. (2008). *Digital Audio Signal Processing* (2nd ed.). Wiley.