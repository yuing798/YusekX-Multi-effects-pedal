---
name: c++代码生成规范
description: 你是一个资深的c++音频开发工程师，在你**生成**代码时，请务必遵循这条规范
---
## 音频性能要求

音频开发对于实时性要求极高


## 命名方式

**宏定义** ：使用大写+下划线，如：SAMPLE_RATE
**函数和类名** ：使用帕斯卡命名法，如：MyName
**普通变量** ：使用驼峰式命名法，如：sampleData
**特殊命名**：类中私有变量函数必须以m开头，静态函数必须以s开头


## 缩进规范

**行规范** ：不要把太多的参数都放在同一行中，要及时换行，如
```c++
    g.fillAll (getLookAndFeel().findColour (
        juce::ResizableWindow::backgroundColourId));
```

## class和函数规范

**构造，析构函数相关**：即使没有任何功能，也需要显式定义为default
**class和struct使用** ：类使用class, 类中模块使用struct， 并合理使用public和private
**类中方法** ：禁止在类中进行函数定义，只允许做函数声明，如：

```c++
class MainComponent  :  public juce::AudioAppComponent,
                        public juce::ChangeListener
{
public:
    MainComponent();
    ~MainComponent();
    void paint (juce::Graphics&) override;
    void resized() override;

private:

};
```

并在类外进行函数定义，如

```c++
void MainComponent::paint (juce::Graphics& g){}
```

**字符输出规范**：只能使用"\n"换行，禁止使用"std::endl ，字符串内容只能使用英文，禁止使用中文"
**命名空间规范**：禁止使用using namespace
**括号的选择**：无论if,else,for,while等条件判断语句有几行，都必须加花括号{}，如

```c++
    if(source == &transportSource){
        if (transportSource.isPlaying()){
            playPauseButton.setButtonText("pause");
        }else{

            
            if(transportSource.getCurrentPosition() == transportSource.getLengthInSeconds()){
                transportSource.setPosition(0.0);
            }
            playPauseButton.setButtonText("play");
        }
            
    }
```

**if条件规范**：禁止使用三元表达式，如：

```c++
float target = isDragging.load() ? 0.1f : 1.0f;
```

应该使用：
```C++
float target;
if(isDragging.load()){
    target = 0.1f;
}else{
    target = 1.0f;
}
```

**变量初始化**：无论是否在类中还是类外的构造函数中初始化，务必使用初始化列表，如

```c++
uint32 lastProgressBarValue = 0;
//修改成：
uint32 lastProgressBarValue{0};
```

**public和private的选择**：在类中private在上，public在下，且private中的变量方法名必须以m开头
**size规则**：二进制数必须使用 "1 << x" 的写法，如 1024 应该修改为 1 << 10
整数类型优先使用 size_t,如果由负数要求再使用int

**变量封装性**：对于同一个模块的变量及函数，应该使用同一个struct封装

```C++
struct mFFTgraphic{
    juce::AudioBuffer<float> fifo;
    int FFTSize{ 1 << 10 }; // 1024 samples
    int FFTOrder{ 10 }; // log2(FFTSize)
    int fftFifoIndex{ 0 };
    bool fftBufferReady{ false };
};
mFFTgraphic mFFTgraphic;
```

**变量类型**：auto关键字只可用于局部作用域（局部作用域指不超过50行的作用域）