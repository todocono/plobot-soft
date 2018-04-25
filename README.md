# Software from Plobot

This version is the up to date. Main file has the date next to it from the latest modification.
Tests are a bit random, beware of messing up with the pinout of the speaker.

Get yours at http://plobot.com



---

Using Arduino Integrated Development Environment 使用Arduino集成开发环境

We wrote the following instructions for an advanced programmer that wants to pursue programming Plobot.  It is fair to say that the current process is fairly complicated and there’s risk of permanently damaging the hardware. For example, incorrect configuration of the audio amplifier will burn the speaker. We mean it, burn and burn, with smoke and everything.
We would love to release a kid-friendly programming environment so that additional Smart Cards for new functions or sequences of movements can be added to Plobots. If you want to help please get in touch with us.

Arduino IDE can be downloaded from http://arduino.cc. It should be compatible with the most current version, 1.8.3 at the time of writing this guide. Beware of using versions earlier than 1.6.5.
Board model has to be downloaded from the Board Manager under Tools/boards menu.
Set in the Preferences menu this JSON file:
https://raw.githubusercontent.com/Lauszus/Sanguino/master/package_lauszus_sanguino_index.json
In Tools/boards manager, let it update and use Sanguino version 1.0.2
Copy all examples and libraries in the *Arduino* folder defined in the Preferences folder setting
Close and reopen Arduino
Make sure you select the right model of board and also clock and model of CPU: Sanguino, 8MHz, ATMega644p. (note the “p” at the end of “ATmega644”, this gave us plenty of headaches)
The most updated version of the source code is hosted at http://github.com/todocono/plobot
Use an example from the Plobot folder to test that arduino compiles correctly. Do not upload, just compile. If there are errors, it’s not loading the libraries properly.


If you are using Arduino to just upload code on top of the bootloader, that would do it. Make sure you install the right drivers. We used this blog article ( https://kig.re/2014/12/31/how-to-use-arduino-nano-mini-pro-with-CH340G-on-mac-osx-yosemite.html ) quite efficiently in our workshops. You should be able to see your Plobot and program it.

To upload with the firmware
Connect XSP programmer ( https://www.dfrobot.com/product-1323.html )
Select AVR ISP programmer in Tools. Select the right port in tools.
Open Serial Monitor and choose 9600bps and end of line and CR. press enter once. After seeing *configuration* response, type “help“ and press enter.
Set the configuration to:
 mode=FTDI
 out=3V3
 dtr=ENABLE
Select from preferences to show verbose when upload
if you want to download bootloader, do  mode=ISP and burn boot loader from tools
Enter again the config mode and select FTDI
Select the right port in tools again. Now you can upload any Plobot code.
If lights alternate ON and OFF on the XSP, the over voltage protection is ON and it will not work. Note that there is a bug that sometimes requires to unplug both things, plug first the programmer and then the Plobot.
If you are going to tune up the PID cycle, we found this article absolutely enlightening: http://brettbeauregard.com/blog/2011/04/improving-the-beginners-pid-introduction/

我们为想要编程Plobot的高级程序员编写了以下指令。平心而论，目前的过程相当复杂，且存在永久损坏硬件的风险。例如，音频放大器的错误配置会烧坏扬声器。我们的意思是，真的燃烧和冒烟。

我们希望发布一款适合儿童的编程环境，以便为Plobots增加新功能或动作序列的智能卡。如果您想帮忙，请与我们联系。

Arduino IDE可以从http://arduino.cc下载。它应该与编写本指南时的最新版本1.8.3兼容。谨防使用1.6.5之前的版本。
电路板型号必须从工具/电路板菜单下的电路板管理器下载。
在偏好菜单中设置这个JSON文件：
https://raw.githubusercontent.com/Lauszus/Sanguino/master/package_lauszus_sanguino_index.json
在工具/电路板管理器中，让它更新并使用Sanguino版本1.0.2。
复制首选项文件夹设置中定义的* Arduino *文件夹中的所有示例和库文件。
关闭并重新打开Arduino。
确保你选择了正确的电路板型号，以及CPU的时钟和型号：Sanguino，8MHz，ATMega644p。 （注意“ATmega644”末尾的“p”，这让我们的头很痛）
源代码的最新版本托管在http://github.com/todocono/plobot
使用Plobot文件夹中的示例来测试arduino是否正确编译。不要上传，只需编译。如果有错误，则不会正确加载库文件。

如果您使用Arduino只是在引导加载程序的顶部上载代​​码，那就可以了。确保您安装了正确的驱动程序。在我们的工作坊中，我们非常有效地使用了这篇博客文章（https://kig.re/2014/12/31/how-to-use-arduino-nano-mini-pro-with-CH340G-on-mac-osx-yosemite.html）。您应该能够看到您的Plobot并对其进行编程。

连接XSP程序员（https://www.dfrobot.com/product-1323.html）
在工具中选择AVR ISP编程器。在工具中选择正确的端口。
打开串行监视器并选择9600bps并结束行和CR按一次输入。看到*配置*响应后，输入“help”并按回车。
将配置设置为：
 model= FTDI
 OUT = 3V3
 DTR = ENABLE
上传时从偏好设定中选择以显示详细信息。
如果你想下载bootloader，请执行mode = ISP并从工具中刻录启动加载器。
再次输入配置模式并选择FTDI。
再次在工具中选择正确的端口。现在您可以上传任何Plobot代码。
如果XSP上的灯交替开和关，则过压保护将开启Plobot将不运作。请注意，有时候需要拔掉这两个东西，先插入程序员，然后再插入Plobot。
如果你打算调整PID周期，我们发现这篇文章绝对有启发性：http://brettbeauregard.com/blog/2011/04/improving-the-beginners-pid-introduction/


