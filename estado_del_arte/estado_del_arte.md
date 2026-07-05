# Estado del arte

El termino **bunny-virus** se relaciona tecnicamente con ataques tipo **rabbit/wabbit** o **fork bomb**, donde un proceso se replica o consume recursos hasta bloquear el sistema.

## 1. Fork Bomb Attack Mitigation by Process Resource Quarantine

Habla directamente de fork bombs. Lo mas relevante es que propone mitigar el ataque aislando procesos que consumen recursos de forma anormal, en lugar de esperar a que el sistema colapse.

## 2. Accurate fork bomb detection by process name

Se enfoca en detectar fork bombs usando informacion del proceso. Para el bunny-virus, aporta una senal simple de identificacion, aunque debe complementarse con metricas de consumo.

## 3. Detection and Prediction of Resource-Exhaustion Vulnerabilities

Estudia vulnerabilidades de agotamiento de recursos. Es importante porque el bunny-virus funciona precisamente agotando memoria, CPU o capacidad de respuesta del sistema.

## 4. Exploring the Possibility of USB based Fork Bomb Attack on Windows Environment

Analiza una fork bomb ejecutada desde USB en Windows. Lo resaltante es que el ataque puede venir de medios externos y crear procesos hasta saturar el sistema.

## 5. Resource Exhaustion

Presenta el agotamiento de recursos como riesgo de software. Para el proyecto, refuerza la necesidad de limites, monitoreo continuo y respuesta temprana.
