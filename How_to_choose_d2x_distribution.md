# How to choose the d2x distribution fitting your needs #

Starting from v10 there are 2 branches of the cios.

## cIOS branches ##

| **Branch** | **Pros** | **Cons** |
|:-----------|:---------|:---------|
| d2x        | - Higher compatibility for nand emulation. | - 1 usb port supported.<br> - Non plug&play usb devices, meaning that you MUST connect them before starting the game/usbloader. <br>
<tr><td> d2x-alt    </td><td> - 2 usb ports supported.<br> - Fully plug&play usb devices, meaning that you can connect them when the game asks for them. </td><td> - Lower compatibility for nand emulation. </td></tr></tbody></table>

## cIOS editions ##
Each branch has 2 editions, one for real Wii and the other one for Virtual Wii inside a WiiU.
So there are a total of 4 distribution files.
```
IMPORTANT NOTE:

Nerver install a vWii edition on a real Wii and viceversa.
```

Let's see an example.

| **Distribution file**    | **d2x**   | **d2x-alt** | **Real Wii** | **Virtual Wii inside a WiiU** |
|:-------------------------|:----------|:------------|:-------------|:------------------------------|
| d2x-v10-final            |   X       |             |     X        |                               |
| d2x-v10-final-vWii       |   X       |             |              |            X                  |
| d2x-v10-final-alt        |           |    X        |     X        |                               |
| d2x-v10-final-alt-vWii   |           |    X        |              |            X                  |


## Final notes ##
Here are some useful recommendations.
  1. Currently ModMii v6.2.3 doesn't support the vWii editions. Use the d2x-cios-installer in this case.
  1. If you have a problematic HDD try changing usbloader and/or d2x branch.
