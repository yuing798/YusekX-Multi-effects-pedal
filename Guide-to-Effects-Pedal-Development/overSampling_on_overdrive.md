# 过载效果器中的过采样技术

在现代音频效果器设计中，**过采样（Oversampling）** 是处理非线性失真时不可或缺的一环。本文将以一个具体的过载效果器（Base Overdrive）为蓝本，深入剖析其过采样信号链：  
**上采样 → 抗镜像 FIR → 非线性失真 → 抗混叠 FIR → 下采样**。

下面从**音频听感**、**信号与系统数学原理** 以及**实际 C++/JUCE 代码实现**三个角度展开，并穿插回答以下核心问题：

1. 为什么插值后会产生镜像频谱？  
2. 为什么上采样之后必须接一个抗镜像 FIR？  
3. 非线性失真为什么会产生高次谐波？  
4. 为什么非线性失真后必须接抗混叠 FIR？  
5. 采用过采样与未使用过采样的过载效果器在听感和信号层面有何差异？  
6. 多相分解（Polyphase Decomposition）如何降低计算量？它在 Noble 恒等式的视角下又提供了什么优势？

---

## 1. 过采样信号链全貌

在 `base_overdrive.cpp` 的 `processBaseOverdrive()` 方法中，我们可以清晰地看到过采样被内嵌在单样本循环里：

```cpp
// 信号放大后进入过采样
float inputSampleLeft = channelDataLeft[sampleIndex] * currentDrive;

// 第一步：上采样 + 抗镜像 FIR（通过多相结构一次性完成）
std::vector<float> boostBufferLeft = overSamplingStateLeft.processUpSamplingMultiPhase(inputSampleLeft);

// 第二步：非线性失真（tanh 逼近）
for(size_t index = 0; index < boostBufferLeft.size(); index++){
    tanhApproximate(boostBufferLeft[index]);
}

// 第三步：抗混叠 FIR + 下采样（一次性完成）
inputSampleLeft = overSamplingStateLeft.processDownSamplingMultiPhase(boostBufferLeft);
```

后续的低通滤波、干湿混合并不属于过采样链路，因此不在本文讨论范围。下面我们就按照信号链的顺序，逐环节展开。

---

## 2. 上采样（Upsampling）与镜像频谱的产生

### 2.1 上采样的数学定义

**上采样** 也称插值，在数字域中通常通过在每两个原始样本之间插入 \( L-1 \) 个零值样本来实现，其中 \( L \) 为过采样倍数。在 `dspFilters.h` 里，我们选用 \( L=4 \)。

记原始输入序列为 \( x[n] \)，上采样 \( L \) 倍后的序列 \( x_e[n] \) 满足：

\[
x_e[n] = 
\begin{cases}
x\left[\frac{n}{L}\right], & n \text{ 是 } L \text{ 的整数倍} \\
0, & \text{其他}
\end{cases}
\]

### 2.2 为什么插零会产生"镜像频谱"？

在 \( \mathcal{Z} \) 域，上采样的效果是：

\[
X_e(z) = X(z^L)
\]

原始信号 \( x[n] \) 的频谱 \( X(e^{j\omega}) \) 在数字角频率 \( \omega \in [-\pi, \pi] \) 上是基带信号。插零后，数字角频率被"压缩"了 \( L \) 倍：基带被映射到 \( \omega \in \left[-\frac{\pi}{L}, \frac{\pi}{L}\right] \)，同时在 \( \omega = \frac{2\pi k}{L} \) 周围出现重复的镜像图像。也就是说，在一个 \( 2\pi \) 周期内会出现 \( L \) 个原始频谱的副本。  

> **直观理解**：采样率提高 \( L \) 倍，信号的真实物理带宽不变，但可利用的数字频带变宽了。原始基带之外的"空余"位置被多个镜像副本占据。

为了让信号在过采样域中仅仅保留干净的低带宽基带，必须使用一个低通滤波器去除所有镜像副本——这就是 **抗镜像 FIR** 的第一个核心作用。

---

## 3. 抗镜像 FIR 滤波器（第一个 FIR）

### 3.1 为什么上采样后必须接抗镜像 FIR？

上采样插零后，信号频谱不再是单一的基带。如果我们直接对这样的信号进行数字‑模拟转换，镜像频率会反映在模拟输出中，产生不想要的超声波成分，甚至可能损坏扬声器。而在本效果器内部，我们还需要继续在过采样域中进行非线性处理；如果带着镜像进行失真，这些镜像会与非线性产生的谐波交叉调制，使得最终的频谱混乱不堪。

因此，抗镜像 FIR 负责：  
- 保留基带信号（截止频率 ≤ 原奈奎斯特频率），  
- 滤除所有镜像副本，使得过采样后的信号在感知上等同于一个以高采样率重新采样的干净信号。

### 3.2 FIR 滤波器设计

在 `dspFilters.h` 中，我们利用 **sinc 函数** 生成理想低通滤波器的冲激响应，再使用 **汉宁窗** 进行加窗，以获得实际可实现的线性相位 FIR。

```cpp
// 从 OverSampling 类中摘录
std::vector<float> OverSampling::setFIRCoefficients(){
    int FIROrders = numCoefficients - 1;          // 阶数 = 系数个数 - 1
    std::vector<float> coefficients(numCoefficients, 0.0f);

    for(int index = 0; index < numCoefficients; index++){
        // sin(π·t) / (π·t)，并在时域上进行半阶移位以保证因果性
        coefficients[index] = sinc(static_cast<float>(index - static_cast<float>(FIROrders)/2));
    }

    // 汉宁窗抑制旁瓣，减少频谱泄漏
    juce::dsp::WindowingFunction<float> window(
        static_cast<size_t>(numCoefficients),
        juce::dsp::WindowingFunction<float>::hann);
    window.multiplyWithWindowingTable(coefficients.data(), static_cast<size_t>(numCoefficients));

    return coefficients;
}
```

这里 `numCoefficients = 64`，过采样倍数 \( L = 4 \)，因此归一化截止频率为 \( \frac{\pi}{4} \)（即原奈奎斯特频率）。理想低通的冲激响应为 \( h[n] = \frac{\sin(\pi n / L)}{\pi n} \)，加上移位与加窗，即得到所需滤波器的系数。

### 3.3 代码实现中的多相结构

为提高实时性能，我们并没有在图领域先插零再卷积，而是直接使用 **多相分解**（在下一节详述）。在上采样函数中我们可以看到：

```cpp
std::vector<float> OverSampling::processUpSamplingMultiPhase(float value){
    std::fill(upSamplingBufferMultiPhase.begin(), upSamplingBufferMultiPhase.end(), 0.0f);
    upSamplingFifoMultiPhase.push(value);   // 将输入样本推入延迟线

    for(int phase = 0; phase < oversamplingFactor; phase++){
        for(int index = 0; index < kernelSize; index++){
            upSamplingBufferMultiPhase[phase] += 
                upSamplingFifoMultiPhase.buffer[
                    getCircularBufferIndex(
                        upSamplingFifoMultiPhase.read - index,
                        upSamplingFifoMultiPhase.buffer.size())]
                * multiPhaseFIRCoefficientsMatrix[phase][index];
        }
    }
    return upSamplingBufferMultiPhase;   // 返回 L 个过采样输出
}
```

此函数每次输入一个样本，输出一个长度为 \( L \) 的向量，既完成了插值，也完成了抗镜像滤波。

---

## 4. 非线性失真与高次谐波的产生

### 4.1 为什么非线性失真会产生高次谐波？

过载效果器的核心是一个**非线性映射函数**，通常具有"软压缩"特性。该项目使用了 `tanhApproximate()` 函数，即双曲正切函数的近似。对一个正弦波 \( A \sin(2\pi f_0 t) \) 应用对称非线性函数 \( f(x) \)，其输出可以通过泰勒级数展开：

\[
\tanh(x) = x - \frac{x^3}{3} + \frac{2x^5}{15} - \dots
\]

代入正弦波后：

\[
\tanh(A\sin\omega t) = A\sin\omega t - \frac{A^3}{3}\sin^3\omega t + \frac{2A^5}{15}\sin^5\omega t - \dots
\]

利用幂次降幂公式，\( \sin^3\omega t \) 中包含 \( \sin 3\omega t \) 分量，\( \sin^5\omega t \) 中包含 \( \sin 5\omega t \) 等，因此输出中出现了输入频率的**奇次倍频**（\( 3f_0, 5f_0, 7f_0, \dots \)）。这就是非线性失真产生高次谐波的数学根源，且对称型失真主要产生奇次谐波，为声音增添"温暖"的染色。

若输入不是纯正弦，而是复杂的乐器信号，则还会产生交调失真，频谱变得更加丰富。

### 4.2 失真在过采样域中的位置

在代码中，非线性处理位于 `processUpSamplingMultiPhase` 之后、`processDownSamplingMultiPhase` 之前，即完全在 \( L \times \) 过采样域中进行。因为此时采样率为原采样率的 4 倍，根据奈奎斯特定理，可无混叠表示的最高频率也提高了 4 倍（例如从 22.05 kHz 提升到 88.2 kHz）。这样，上述 \( 3f_0, 5f_0 \) 等高次谐波即使超出原奈奎斯特频率，只要不超过新的奈奎斯特频率，都可以被正确表示。

---

## 5. 抗混叠 FIR 滤波器（第二个 FIR）

### 5.1 为什么非线性失真后必须接抗混叠 FIR？

虽然非线性处理在过采样域中能够得到精确的高频谐波，但我们最终仍要将信号降采样回原始的采样率。此时，所有超过原奈奎斯特频率 \( f_s/2 \) 的高频成分若不做滤除，就会**混叠**到可听频率范围，造成刺耳的"数字失真"。

具体而言，下采样（抽取）是上采样的对偶过程：每次只保留每 \( L \) 个样本中的一个，丢弃其余样本。在频域上，这相当于将频谱压缩 \( L \) 倍，并产生周期延拓。如果信号中包含高于 \( f_s/2 \) 的能量，这些能量会被反射回低频，产生非谐波关系的混叠分量，严重劣化音质。

因此，在下采样之前，必须用一个数字低通滤波器将带宽限制在 \( \le f_s/2 \) 以内——这就是 **抗混叠 FIR**。

在项目中，我们使用的正是与上采样相同的 FIR 系数（多相分解版本），但工作方向相反：在 `processDownSamplingMultiPhase` 中同时完成抗混叠滤波与抽取。

---

## 6. 下采样（Downsampling）的实现

`processDownSamplingMultiPhase` 负责接收过采样域中多达 \( L \) 个样本，输出一个降采样后的样本：

```cpp
float OverSampling::processDownSamplingMultiPhase(std::vector<float>& inputBuffer){
    downSamplingFifoMultiPhase.pushBuffer(inputBuffer);

    float outputSample = 0.0f;
    for(int phase = 0; phase < oversamplingFactor; phase++){
        float phaseOutput = 0.0f;
        for(int index = 0; index < kernelSize; index++){
            phaseOutput += downSamplingFifoMultiPhase.buffer[
                getCircularBufferIndex(
                    downSamplingFifoMultiPhase.read - index * oversamplingFactor - (oversamplingFactor - 1 - phase),
                    downSamplingFifoMultiPhase.buffer.size())]
                * multiPhaseFIRCoefficientsMatrix[phase][index];
        }
        outputSample += phaseOutput;
    }
    return outputSample;
}
```

这里读取延迟线的索引经过精心设计，保证每个相位分支只处理该相位对应的多相滤波运算，所有分支相加即得到最终的下采样样本。同时该样本已经经过了抗混叠过滤，不包含超过 \( f_s/2 \) 的混叠风险。

---

## 7. 过采样对听感的改善

| 特性 | 无过采样的过载 | 有过采样的过载 |
|------|----------------|----------------|
| **谐波表现** | 高次谐波（>22.05 kHz）会混叠回低频，导致不和谐的杂散频谱 | 高次谐波在过采样域中正确生成，然后被数字低通滤除，不产生混叠 |
| **听感** | 声音容易显得"脏"、"沙哑"，尤其在高增益下可能出现金属感或数字噪声 | 声音更平滑、自然，失真特性类似模拟设备，仅保留自然的谐波染色 |
| **冲击响应** | 混叠伪影使得瞬态边缘模糊，立体声像可能受影响 | 瞬态清晰、紧凑，声像稳定 |

在信号与系统的角度，这是 **扩展动态范围** 与 **消除混叠** 的直接体现。虽然过采样不改变原信号的量化精度，但它允许我们在数字内部"安全地"产生远超原奈奎斯特频率的信息，然后再强制带限回原频段，从而避免了混叠失真。

---

## 8. 多相分解与 Noble 恒等式：计算量的秘密

### 8.1 为什么需要多相分解？

直接实现上采样 + FIR 滤波的流程需要：
1. 将每个输入样本扩展为 \( L \) 个样本（其中 \( L-1 \) 个为零），  
2. 与长度 \( N \) 的 FIR 卷积。  

对于每个输入样本，这需要约 \( N \) 次乘加运算（因为只需在非零位置计算）。看似不多，但当 \( N=64 \)、\( L=4 \) 且实时音频流每秒有 44 100 个输入样本时，每声道每秒约需 \( 64 \times 44100 = 2.82 \times 10^6 \) 次乘加，尚可接受，但如果要设计更陡峭的滤波器（更大 \( N \)），计算量将线性增长。

**多相分解**（Polyphase Decomposition）利用 **Noble 恒等式**，将滤波与插值/抽取的顺序互换，从而将卷积运算放在低采样率一侧进行。这样总运算量约为 \( \frac{N}{L} \times L = N \) 次乘加（在此例中相同），但优势在于：
- 结构上更简洁，缓存友好；
- 当 \( L \) 较大时，若使用对称 FIR，多相分解后每个子滤波器的系数减少，且可利用系数对称性进一步减半；
- 允许我们直接在高、低采样率双域中分别用循环缓冲区实现，内存占用固定。

在 `OverSampling` 类中，`multiPhaseFIRCoefficientsMatrix` 存放了 \( L \times (N/L) \) 的系数矩阵，每个子滤波器负责一个相位。上采样时，每个相位分支独立计算出一个输出样本；下采样时，所有相位分支的输出求和即得最终结果。

### 8.2 Noble 恒等式速览

Noble 恒等式描述了 **上/下采样** 与 **滤波** 的交换规则：

- **恒等式一**：一个 \( L \) 倍上采样器后接传递函数为 \( H(z) \) 的滤波器，可以等效为：先对输入进行滤波 \( H(z^L) \)，然后再上采样。  
- **恒等式二**：一个传递函数为 \( H(z) \) 的滤波器后接一个 \( L \) 倍下采样器，可以等效为：先下采样，再用 \( H(z^{1/L}) \) 滤波（须分解为多相形式实现）。

多相分解正是将 \( H(z) \) 按照 \( z^{-k} \) 分解为 \( L \) 个分支，再应用 Noble 恒等式，从而将滤波器移到采样率转换器的另一边，大幅降低运算量。我们的代码实现直接映射了这一原理。

---

## 9. 总结

过载效果器中的过采样信号链可概括为：

1. **上采样（插零）** → 产生镜像频谱  
2. **多相抗镜像 FIR** → 滤除镜像，保留干净基带  
3. **非线性失真** → 在扩展的采样率下生成丰富的高次谐波  
4. **多相抗混叠 FIR** → 限制带宽，防止降采样时混叠  
5. **下采样（抽取）** → 回到原始采样率，保持最佳音质  

通过 `dspFilters.h` 中的 `OverSampling` 类，我们将数学原理转化为高效、实时的 C++ 实现。理解这一链路不仅能帮助开发者写出更专业的音频 DSP 代码，也能让音乐人理解为什么那些模拟味十足的效果器背后，往往藏着精巧的过采样设计。

> **附录：代码文件索引**  
> - `base_overdrive.h / .cpp`：定义过载效果器，内部调用过采样 API  
> - `dspFilters.h`：`OverSampling` 类，实现多相结构 FIR 过采样/降采样  
> - `dataStructs.h`：循环缓冲区 `FIFO`  
> - `mathFunc.h / .cpp`：sinc 函数、环形索引、线性插值等基础数学工具  

