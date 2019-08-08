[//]: # (For development of this README.md, use http://markdownlivepreview.com/)

# PhaserCHOP

## What is PhaserCHOP?
PhaserCHOP is an easy-to-use tool for staggering animations. Suppose you have several identical items that need to be animated with the LookupCHOP, but you don't want all items to start and stop at the same time. If an item should start ahead of the pack and end ahead of the pack, then that item has a higher phase close to 1. Items that start late and end late have a lower phase close to 0. Phase values must be between 0 and 1. Connect a CHOP with one channel and any number of phase values to the first input of the PhaserCHOP. Next connect an LFO that has been set to "ramp" as the second input to the PhaserCHOP. As the LFO rises, observe how samples with higher phase move from 0 to 1 earlier than samples with lower phase. Now adjust the "Edge" parameter on the PhaserCHOP to your preference. Notice how when the LFO is 0, all output from the PhaserCHOP is 0, and how when the LFO is 1, all output from the PhaserCHOP is 1. This is true regardless of your choice of the "Edge" parameter.

## Instructions
[Build the dll yourself](https://docs.derivative.ca/Write_a_CPlusPlus_Plugin), or use of the compiled DLL files. `CPlusPlusCHOP/build/PhaserCHOP.dll` has been compiled for TouchDesigner 2018.26750 and `CustomOperator/build/PhaserCHOP.dll` has been compiled for TouchDesigner 2019.17550. This newer version can be used as a [Custom Operator](https://docs.derivative.ca/Custom_Operators)

## Changelog
* 2019-08-07 create a Custom Operator version for TD 2019.17550.
* 2019-07-04 working version.

## Thanks
* [Derivative, makers of TouchDesigner](http://derivative.ca)