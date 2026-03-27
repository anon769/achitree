#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "raylib.h"
#include <math.h>
#include <vector>
#include "types.h"
#include "raymath.h"

// sistema simples de vento (estado global)
struct WeatherSystem {
    float windTime;        // tempo acumulado (usado na oscilação do vento)
    float windStrength;    // força atual do vento
    float targetWind;      // força alvo (para transição suave)
    float windChangeTimer; // tempo até mudar o alvo do vento
};

// inicialização padrão do clima
inline WeatherSystem gWeather = { 0.0f, 0.1f, 0.2f, 0.0f };

// atualiza o comportamento do vento ao longo do tempo
inline void UpdateWeather(float dt) {
    gWeather.windTime += dt;
    gWeather.windChangeTimer -= dt;
    
    // quando o timer acaba, escolhe uma nova intensidade de vento
    if (gWeather.windChangeTimer <= 0) {
        gWeather.targetWind = (float)GetRandomValue(0, 60) / 100.0f; // 0.0 → 0.6
        gWeather.windChangeTimer = (float)GetRandomValue(5, 12);     // próximo ajuste em alguns segundos
    }
    
    // interpola suavemente até o valor alvo
    gWeather.windStrength = Lerp(gWeather.windStrength, gWeather.targetWind, dt * 0.3f);
}

// calcula o deslocamento causado pelo vento em um ponto
inline Vector2 GetWindOffset(Vector2 pos, float heightThreshold, NodeType type, const std::vector<Node>& nodes) {
    
    // não aplica vento em partes baixas ou estruturas fixas
    if (pos.y > heightThreshold + 5.0f || type == ROOT || type == TRUNK)
        return { 0, 0 };
    
    // calcula a distância até o tronco mais próximo
    float minTrunkDist = 9999.0f;
    for (const auto& node : nodes) {
        if (node.type == TRUNK) {
            float d = Vector2Distance(pos, node.position);
            if (d < minTrunkDist) minTrunkDist = d;
        }
    }

    // quanto mais longe do tronco, mais o vento afeta
    float distanceFactor = minTrunkDist * 0.12f;

    // velocidade da oscilação (depende da força do vento)
    float speed = 0.5f + (gWeather.windStrength * 4.0f);

    // intensidade do movimento
    float amplitude = gWeather.windStrength * distanceFactor;
    
    // limita o deslocamento máximo
    if (amplitude > 25.0f) amplitude = 25.0f;
    
    // movimento oscilatório horizontal (efeito de balanço)
    float sway = sinf(gWeather.windTime * speed + (pos.x * 0.02f)) * amplitude;

    return { sway, 0 };
}

#endif
