# AI_Reversi

一个用C++编写的黑白棋程序。程序默认读取根目录中的Hereditary.txt(需自行创建)的内容作为参数。Hereditary.txt中没有多余的换行，每行一个小数，分别对应以下参数：

* type：参数的类型。为0或者为1时使用全局评估方法，以下参数使用；为2时使用模块评估方法，
* move_initial_power：行动力差距造成的初始影响力。
* move_diff_power：行动力差距造成的影响力。
* corner_power：角落子的影响力。
* edge_power：边缘子的影响力。
* near_corner_power：靠近角落子的影响力。
* near_edge_power：靠近边缘子的影响力。
* stable_power：稳定子的影响力。
* fake_stable_power：伪稳定子的影响力。
* unstable_power：不稳定子的影响力。
* non_current_move_power：非当前行动对象的行动力修正值。
* importance_power：棋子的影响力。

当参数为2时，以下的参数为每个模块的权重。