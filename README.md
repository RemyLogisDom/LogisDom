# LogisDom
![Logo LogisDom](https://user-images.githubusercontent.com/93469884/198893669-c48d0f25-6a75-4c48-a8e3-13cca6277102.png)


LogisDom is a WYSIWYG software for home supervision and control.

It is like a domotic software, but more oriented for automation and specially multi energy control within a house.

It is mainly based on 1 wire, ModBus, TeleInfo. Other communication protocols are also supported, like EnOcean and M-Bus.

All these protocols are driven via ethernet interface, USB and COM port are not the prefered connections even if it could be implemented. Only EnOcean can be used via USB port.

LogisDom displays and stores all the sensors values with complete customized GUI layout, it can show graphics, perform calculations to operate automation, for example starting the charge of a electric car when photovoltaic pannels are giving enough power and when the battery level is high enough, or manage a heat pump for proper room temperature control.

examples here https://remylogisdom.github.io

LogisDom can run under PC Windows & Linux and Rapsberry Pi.


How to build with Qt 5.15.2

LogisDom needs Quazip and Qwt

Actual Quazip version used is 0.7.3, and Qwt is 6.0.2

In your Qt build project (for example /LogisDom/build-LogisDom-Desktop_Qt_5_15_2_GCC_64bit-Release)

you need to copy the Quazip source code in a folder named quazip-0.7.3, and comppile it

For Qwt it has to be copied in a folder named qwt-6.2.0

compiled library file must be stored in a folder named lib under Qt build folder.

You can do differently by modifying the LogisDom.pro file.

LogisDom should compile straight forward doing like this.
