## 关于项目/About This Project

将.hdr文件转化为立方体贴图，并对该贴图进行卷积，以计算漫反射辐照度。

convert .hdr file to cube map, then convolute that map to calculate irradiance from all directions for single $\omega_i$.

## Inference(from environment map to irradiance map)

rendering function in real time rendering:
$$
L_o(p, \omega_o)=\int_{\Omega}(k_d \frac{C}{\pi}+k_s\frac{DFG}{4(\omega_o\cdot n)(\omega_i\cdot n)})L_i(p_i, \omega_i)n\cdot\omega_i d\omega_i
$$
for the diffuse part, only depends on $\omega_i$:
$$
L_o(p, \omega_o)=k_d\frac{C}{\pi}\int_{\Omega}L_i(p_i, \omega_i)n\cdot\omega_i d\omega_i
$$
So rather than calculating the integral online(by Monte Carlo), we can pre-compute this offline, then use$f(\omega_o)=\int_{\Omega}L_i(p_i, \omega_i)n\cdot\omega_i d\omega_i$ to represent the integral.
$$
L_o(p, \phi_o, \theta_o)=k_d\frac{C}{\pi}\frac{1}{n_1n_2}\sum^{n_1}_{\phi=0}\sum^{n_2}_{\theta=0}L_i(p, \phi, \theta)cos\theta sin\theta
$$


We use cube map to storage the result of pre-computing integral, name it "irradiance map".

