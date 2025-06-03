#include <iostream>
#include "ludo.h"

void mostrar_tablero() {
    unsigned int* tableroPtr = obtener_tablero();  // Obtener puntero al tablero desde la DLL

    std::cout << "\n=== ESTADO DEL TABLERO ===\n";
    for (int i = 0; i < 52; ++i) {
        if (tableroPtr[i] == 0)
            std::cout << ".";
        else
            std::cout << tableroPtr[i];  // número de jugador (1–4)

        std::cout << " ";
        if ((i + 1) % 13 == 0)
            std::cout << "\n";
    }

    std::cout << "\nFICHAS POR JUGADOR:\n";
    for (int jugador = 0; jugador < 4; ++jugador) {
        std::cout << "Jugador " << jugador + 1 << ": ";

        void* ficha0 = obtener_ficha_jugador(jugador, 0);
        void* ficha1 = obtener_ficha_jugador(jugador, 1);

        int pos0 = *(int*)ficha0;
        int pos1 = *(int*)ficha1;

        std::cout << "Ficha 0 en ";
        if (pos0 == -1) std::cout << "CASA";
        else if (pos0 == 999) std::cout << "META";
        else std::cout << "pos " << pos0;

        std::cout << ", Ficha 1 en ";
        if (pos1 == -1) std::cout << "CASA";
        else if (pos1 == 999) std::cout << "META";
        else std::cout << "pos " << pos1;

        std::cout << "\n";
    }
}

int main() {
    std::cout << "\n=== INICIO DEL JUEGO DE LUDO ===\n";

    inicializar_juego();

    bool terminado = false;
    int turno = 0;

    while (!terminado) {
        if (turno < 0 || turno > 3) {
            std::cerr << "[ERROR] Turno inválido: " << turno << "\n";
            break;
        }

        std::cout << "\n-- Turno del jugador " << turno + 1 << " --\n";

        int dado = lanzar_dado();
        std::cout << "Lanzó el dado: " << dado << "\n";

        void* ficha0 = obtener_ficha_jugador(turno, 0);
        void* ficha1 = obtener_ficha_jugador(turno, 1);

        bool puede0 = ficha0 && puede_mover_ficha(ficha0, dado);
        bool puede1 = ficha1 && puede_mover_ficha(ficha1, dado);

        if (puede0 || puede1) {
            int eleccion = 0;

            if (dado == 6 && puede0 && puede1) {
                std::cout << "Ambas fichas pueden moverse. Elige cuál mover (0 o 1): ";
                std::cin >> eleccion;
                if (eleccion != 0 && eleccion != 1) {
                    std::cout << "Entrada inválida. Se moverá ficha 0 por defecto.\n";
                    eleccion = 0;
                }
            }
            else if (puede0) {
                eleccion = 0;
            }
            else {
                eleccion = 1;
            }

            std::cout << "Moviendo ficha " << eleccion << " del jugador " << turno + 1 << "\n";
            mover_ficha(turno, eleccion, dado);
        }
        else {
            std::cout << "No hay movimientos posibles.\n";
        }

        if (verificar_victoria(turno)) {
            std::cout << "\n*** ¡Jugador " << turno + 1 << " gana el juego! ***\n";
            terminado = true;
        }

        mostrar_tablero();

        if (dado != 6)
            turno = (turno + 1) % 4;

        system("pause");
    }

    std::cout << "\n=== FIN DEL JUEGO ===\n";
    return 0;
}
