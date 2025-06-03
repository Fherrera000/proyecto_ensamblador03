#pragma once

extern "C" {
    void inicializar_juego();
    int lanzar_dado();
    void* obtener_ficha_jugador(int jugador, int numFicha);
    int puede_mover_ficha(void* ficha, int dado);
    void mover_ficha(int jugador, int ficha_idx, int dado);
    void sacar_ficha_de_casa(void* ficha, int jugador);
    void capturar_ficha(void* ficha, int jugador);
    void verificar_captura(int jugador, int posicion);
    int verificar_victoria(int jugador);
    unsigned int* obtener_tablero(); // <-- agregado
}
