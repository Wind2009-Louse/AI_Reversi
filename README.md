# AI_Reversi

一个用C++编写的黑白棋程序。可以通过根目录的Hereditary.txt(需自行创建)设定参数。Hereditary.txt共11行(没有多余的换行)，每行一个小数，分别对应以下参数：

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