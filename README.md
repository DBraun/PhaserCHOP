[//]: # (For development of this README.md, use http://markdownlivepreview.com/)

# PhaserCHOP
Browse the examples from David Braun's ["Quantitative Easing"](https://github.com/DBraun/PhaserCHOP-TD-Summit-Talk) Talk at the 2019 TouchDesigner summit.

## The GLSL Function
```
float phaser(float pct, float phase, float e) {
    return clamp( (phase-1.+pct*(1.+e))/e, 0., 1.);
}
```
Phaser is a parameterized easing function for phase-staggered animations. The arguments:

* pct : [0,1]
* phase : [0,1]
* e (edge) : > 0

These are the characteristics of the function:

* When pct is 0, phaser returns 0, regardless of the other variables.
* When pct is 1, phaser returns 1, regardless of the other variables.
* All else equal, a larger phase causes phaser to return 1 sooner.
* All else equal, a larger edge causes differences in phase to matter less.
* All else equal, a smaller edge causes differences in phase to matter more. (Corollary to previous)


## Instructions
[Build the dll yourself](https://docs.derivative.ca/Write_a_CPlusPlus_Plugin), or use of the compiled DLL files. `CPlusPlusCHOP/build/PhaserCHOP.dll` has been compiled for TouchDesigner 2018.26750 and `CustomOperator/build/PhaserCHOP.dll` has been compiled for TouchDesigner 2019.17550. This newer version can be used as a [Custom Operator](https://docs.derivative.ca/Custom_Operators)

## Changelog
* 2019-08-07 create a Custom Operator version for TD 2019.17550.
* 2019-07-04 working version.

## Thanks
* [Derivative, makers of TouchDesigner](http://derivative.ca)
