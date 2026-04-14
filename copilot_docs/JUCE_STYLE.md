---
name: JUCE项目函数使用规范
description: 你是一个资深的JUCE开发工程师，在你调用JUCE类型及代码时，请务必遵循这条规范
---

## GUI开发相关

### resized函数

在MainComponent.h中，优先使用juce::rectangle<int> 进行布局

然后在MainComponent.cpp中的resized函数中，依照示例规范在juce::rectangle定位的空间中添加功能,直接使用绝对坐标进行定位。resized函数只做图形位置确定和组件绑定

```c++
    auto bounds = getLocalBounds();

    mInputDeviceRect = juce::Rectangle<int>(10,10,200,50);
    mInputDeviceBox.setBounds(mInputDeviceRect);

    mPlayPause.rect = juce::Rectangle<int>(220,10,50,50);
    mPlayPause.button.setBounds(mPlayPause.rect);

    mVolumeModify.rect = juce::Rectangle<int>(280,10,400,50);
    mVolumeModify.slider.setBounds(mVolumeModify.rect.removeFromRight(320));
    mVolumeModify.label.setBounds(mVolumeModify.rect.removeFromLeft(80));

    mTremolo.area = juce::Rectangle<int>(10,70,1000,200);
    mTremolo.resize(mTremolo.area);
    
    mFFTgraphic.spectrumRect = juce::Rectangle<int>(0,570,1200,200);

```

除非像下面的大组件内只有两个小组件，否则不要使用removeFrom ... 函数

```c++
    mVolumeModify.rect = juce::Rectangle<int>(280,10,400,50);
    mVolumeModify.slider.setBounds(mVolumeModify.rect.removeFromRight(320));
    mVolumeModify.label.setBounds(mVolumeModify.rect.removeFromLeft(80));
```

**美观性要求**：能看出这个模块是什么功能就行，不要使用如reduced之类的函数，不要有任何美观性

**字体要求**：不需要使用setFont函数，直接默认字体即可

## 音频变量，类型，函数相关

不要使用juce::AudioSampleBuffer,要使用juce::AudioBuffer<float>

