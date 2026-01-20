## sandbox

### about

Grid based and manually customized "physics“

| Element A | Element B                | Interaction Behavior                                                                                                    |
| --------- | ------------------------ | ----------------------------------------------------------------------------------------------------------------------- |
| **Fire**  | **SAND**                 | - Fire can be placed on SAND<br>- SAND adjacent to FIRE may ignite (12.5% per frame)<br>- Burning SAND remains in place |
| **Fire**  | **STONE**                | - Fire can be placed on STONE<br>- STONE adjacent to FIRE may ignite (6.7% per frame)                                   |
| **Fire**  | **EMPTY**                | - Fire **cannot** be placed on EMPTY<br>- EMPTY **cannot** be ignited                                                   |
| **Fire**  | **WATER**                | - WATER extinguishes FIRE on contact (both disappear)<br>- FIRE cannot spread through WATER                             |
| **SAND**  | **WATER**                | - SAND sinks through WATER (swaps positions)<br>- SAND falls normally through EMPTY                                     |
| **SAND**  | **FIRE**                 | - SAND ignites when near FIRE <br>- Burning SAND stops falling                                                          |
| **WATER** | **EMPTY**                | - WATER flows down, then diagonally, then horizontally                                                                  |
| **WATER** | **STONE / SAND (solid)** | - WATER flows around solids                                                                                             |
| **SMOKE** | **EMPTY**                | - SMOKE rises upward with random horizontal drift (turbulence)<br>- May dissipate naturally (2% per frame)              |
| **SMOKE** | **Solids (SAND/STONE)**  | - SMOKE flows around obstacles (diagonally/horizontally when blocked)                                                   |

### demo

![](https://github.com/cryer/hako/raw/master/docs/res/demo.gif)

### blog

了解更多流体仿真，查看我的博客:

[流体基础](https://cryer.github.io/2018/11/fluidnotes/)

[SPH流体仿真实现](https://cryer.github.io/2018/12/fluids/)
