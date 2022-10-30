# LogisDom

LogisDom is a WYSIWYG software for home supervision and control.

It is not a domotic toy, it is more oriented to automation for multi energy control.

It is mainly based on 1 wire, ModBus, TeleInfo. Other communication protocols are also supported, like EnOcean and M-Bus.

All these protocol are driven via ethernet interface, USB and COM port are not the prefered connections even if it could be implemented. Only EnOcean can be used via USB port.



How to build with Qt 5.15.2

LogisDom needs Quazip and Qwt

Actual Quazip version used is 0.7.3, and Qwt is 6.0.2

In your Qt build project (for example /LogisDom/build-LogisDom-Desktop_Qt_5_15_2_GCC_64bit-Release)

you need to copy the Quazip source code in a folder named quazip-0.7.3, and comppile it

For Qwt is it in a folder named qwt-6.2.0

compiled library file must be stored in a folder named lib

You can do differently as you wish by modifying the LogisDom.pro file.

LogisDom should compile straight forward like this.
