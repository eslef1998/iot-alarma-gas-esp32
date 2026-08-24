# Interpretación - IOT-EF4EEA4151

En un laboratorio de prototipado donde se trabaja con diferentes materiales, siempre existe el riesgo de que ocurra alguna fuga de gas combustible. La idea con este proyecto es armar un sistema sencillo que detecte esa condición de peligro a tiempo, permitiendo que los estudiantes o el personal del laboratorio reaccionen antes de que suceda un accidente

El flujo del prototipo es claro: el sensor MQ2 mide el nivel de gas y le manda la lectura analógica al ESP32 y El microcontrolador evalúa ese dato cada 1450 ms para decidir qué hacer. Muestra la medición y el estado actual en la pantalla LCD 1602 y activa el buzzer únicamente si detecta una amenaza real

Para no disparar la alarma por una variación rápida o ruido en la señal, la regla exige que el valor supere el umbral de 2650 ADC durante 6 lecturas consecutivas. Solo cuando se cumplen esas 6 confirmaciones suena el buzzer. Igualmente, la alarma no se apaga apenas el gas baje un poco, sino que debe caer por debajo de 2646 ADC . Mantener el buzzer en estado seguro y evita generar pánico o ruidos molestos innecesarios.

supuestos:
1. La alimentación eléctrica del ESP32 se mantiene estable mientras se usa.
2. La señal del sensor simulado en Wokwi reacciona de forma coherente con el nivel de gas.

Limitaciones:
1. Es un circuito totalmente local, no manda alertas al celular ni se conecta a internet.
2. Es un prototipo académico de prueba y no un detector industrial certificado.