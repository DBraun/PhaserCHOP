[//]: # (For development of this README.md, use http://markdownlivepreview.com/)

# PhaserCHOP

## Background

Let's review a basic easing function often called "ease-in".
```glsl
float easeIn(float pct) {
  return pct * pct;
}
```
Immediately we should note that `easeIn(0)==0` and `easeIn(1)==1`. This is a normal requirement of easing functions. You give it 0; you get 0. You give it 1; you get 1. All of the animation in between is up to you, either through keyframing or designing more [complex](https://github.com/glslify/glsl-easings/blob/master/bounce-out.glsl) [equations](https://en.wikipedia.org/wiki/Smoothstep#Generalization_to_higher-order_equations).

What if we had a more generalized approach to easing where multiple inputs could go through the same animation but with staggered offsets? Such a function should be easy to use. The first input should be like `pct`. When you give it 0, the function should return 0 no matter what. When you give it 1, it should return 1 no matter what. And the other parameters should represent the phase identities of the many inputs as well as an overall "phase-separation-severity-kinda-thing". And now...

## The Phaser GLSL Function
```glsl
float phaser(float pct, float phase, float e) {
    return clamp( (phase-1.+pct*(1.+e))/e, 0., 1.);
}
```

* `pct`: [0,1]
* `phase`: [0,1]
* `e` (edge): (0, infinity]

Quick description:

* `pct` (short for "percent") is like the `pct` in `easeIn`. It should be between 0 and 1 inclusive.
* For `phase`, imagine an animation function `f(pct)` and another `f(pct+phase)`. If `phase` is larger, the sample is further down the animation timeline.
* For edge, imagine two different phase values `A` and `B` where `A`<`B`. Now imagine an animation such as an ease-in function. Neither `A` nor `B` has started. Because `B` has the larger phase, it can animate through the ease-in function for a duration `D`. At exactly the moment `B` finishes, begin to animate `A` through the same animation function for the same duration `D`. Because `B` finishes at exactly the moment `A` begins, the difference between `A` and `B` is defined as the edge size `e`. The trick of the phaser function is that we get to play with the `e` parameter and the `phase` values rather than think about the `D` duration value. Instead of `D`, our timeline's overall duration comes from how long we take to animate `pct` from 0 to 1.

The properties of `phaser` can also be summed up like this:

* When pct is 0, phaser returns 0, regardless of the other variables.
* When pct is 1, phaser returns 1, regardless of the other variables.
* All else equal, a larger phase causes phaser to return 1 sooner.
* All else equal, a larger edge causes differences in phase to matter less.
* All else equal, a smaller edge causes differences in phase to matter more. (Corollary to previous)

## Instructions
For a quick start, get `PhaserCHOP.dll` from the [Releases](https://github.com/DBraun/PhaserCHOP/releases) and place it into this repo's `Plugins` folder. To build the file yourself, open `PhaserCHOP.sln` and press `F5` in either Debug mode or Release Mode. A post-build event will copy the newly built DLL into `Plugins`.

The `PhaserCHOP.toe` in this repo is meant to be a unit test. For more interesting examples, check out [https://github.com/DBraun/PhaserCHOP-TD-Summit-Talk](https://github.com/DBraun/PhaserCHOP-TD-Summit-Talk) and David Braun's ["Quantitative Easing" 2019 TouchDesigner Summit Talk](https://www.youtube.com/watch?v=S4PQW4f34c8).

## Changelog
* 2020-06-01 Better unit test `SmoothstepCHOP.toe` for Custom Operator. Build with `TouchDesigner 2020.22080`. Cleanup repo by removing non-custom operator. All releases are now on the [Releases](https://github.com/DBraun/PhaserCHOP/releases) page.
* 2019-08-07 create a Custom Operator version for TD 2019.17550.
* 2019-07-04 working version.

## Thanks
* [Derivative, makers of TouchDesigner](http://derivative.ca)
