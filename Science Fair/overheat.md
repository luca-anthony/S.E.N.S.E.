To ensure that the device would not overheat and harm the user, we thought of many things that could happen, including overheating due to air temperature, magnetic fields, and in case it touched something hot. Below is the math we used to ensure overheating would not happen. For this, we mainly used Maxwell's equations.

$$P = I^2 R \qquad B = \frac{\mu_0 I}{2\pi r} \qquad P_{\text{net}} = \epsilon\sigma A(T_{\text{source}}^4 - T_{\text{cap}}^4)$$

$$I = 0.015 \qquad T_{\text{source}} = 318.15 \qquad T_{\text{cap}} = 218.15 \qquad A = 4.9 \times 10^{-5}$$

$$B = \frac{(4\pi 10^{-7})0.015}{2\pi 0.003} \approx 6.67 \times 10^{-6} \text{ Tesla }(0.066 \text{ Gauss})$$

$$P_{\text{net}} = 0.09(5.67 \times 10^{-8}) \times (4.9 \times 10^{-5}) \times (318.15^4 - 218.15^4)$$

$$P_{\text{net}} \approx 5.9 \times 10^{-6} \text{ Watts }(5.9\mu\text{w})$$
