# LPV

## 理论部分

球谐的理论部分见[自己写的博客](http://cjhxs.yiwanglm.xyz/2022/03/28/coding/CG/%E6%96%87%E6%91%98%E9%98%85%E8%AF%BB/spherical-harmonic-lighting_1/#more)，LPV的理论部分见[CSDN上的文章](https://blog.csdn.net/qq_35312463/article/details/119939806?spm=1001.2014.3001.5501)，其中光照传播的部分公式比较杂乱，我重新推导了一遍：

我们已知光源格中心O点的球谐系数$L_i$，现在要将光源格的光源传输到其左边的格子中，输出也同样为左边格中心A点的球谐系数$L'_i$，示意图如下：

![image-20220511190336054](http://124.222.23.180/i/2022/05/11/627b980fac897.png)

其中，X点为UP面的中心点，$\vec N_x$为X点的法线，易知$\vec N_x$通过A点。

首先由**球谐函数的重建**公式$\widetilde{f(x)}=\sum^{n^2}_{i=0}c_iy_i(x)$，将球谐函数重建，代入OX的方向向量$\vec w_{ox}$：
$$
L(O, \vec w_{xo})=\sum^{n^2}_{i=0}L_iy_i(\vec w_{xo})\;\;\;\;\;(1)
$$
其中，$n^2$为球谐频段索引的平方，一般频段索引为2或3，本项目设为2；$L_i$为光源格的球谐系数；$y_i(x)$为球谐函数。

由于对称性得：
$$
L(O, \vec w_{xo})=L(X, \vec w_{ox})\;\;\;\;\;(2)
$$


由反射方程$L(p, \vec ω_i)=\int_{H^2}f_r(p, \vec ω\to\vec ω_i)L(p, \vec ω)|cos\theta|dw$，代入X点，$\vec w_{ox}$为入射光线，$\vec N_x$为反射光线，得：
$$
\begin{aligned}L(X, \vec N_x)&=\int_{H^2}f_r(X, \vec w\to\vec N_x)L(X, \vec w)<\vec w, \vec N_x>d\vec w，由于只有一处入射光线，得\\
&=f_r(X, \vec w_{ox}\to\vec N_x)L(X, \vec w_{ox}w)<\vec w_{ox}, \vec N_x>，代入兰伯特反射模型，得\\
&=\frac{\rho}{\pi}L(X, \vec w_{ox}w)<\vec w_{ox}, \vec N_x>，其中\rho为菲涅尔系数，由于完全反射，得\rho=1\\
&=\frac{1}{\pi}L(X, \vec w_{ox}w)<\vec w_{ox}, \vec N_x>
\end{aligned}\;\;\;\;\;(3)
$$
UP面所有的点都可以由以上的(1)、(2)、(3)得出，最后对UP面上的点做积分，得：
$$
\begin{aligned}L_{UP\to A}&=\int_{UP}L(x',\vec w_{x'A})dx'，点x'为UP面上任意一点，\vec w_{x'A}为x'到A的方向向量\\\
&\approx A_rL(X, \vec N_x)，A_r为图上所标注的体积角，可经计算得出，注意最右侧面的体积角大小与上下前后面不同
\end{aligned}\;\;\;\;\;(4)
$$
最后得出所有5个面到点A的辐亮度之和即是点A的辐亮度，用球谐函数表示如下，其中$\vec N$表示该面对应的法线，$y_i(x)$表示球谐函数：
$$
L'_i=L_{up,i}y_i(N_x)+L_{down,i}y_i(N_y)+L_{front,i}y_i(N_z)+L_{back,i}y_i(N_j)+L_{right,i}y_i(N_k)\;\;\;\;\;(5)
$$
