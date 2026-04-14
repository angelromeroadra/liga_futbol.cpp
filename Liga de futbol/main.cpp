#define _CRT_SECURE_NO_WARNINGS

// ============================================================
//  Liga de Futbol - Proyecto C++
//  Compilar: g++ -o liga main.cpp
//  Ejecutar: ./liga  (Linux/Mac) o liga.exe (Windows)
// ============================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>

using namespace std;

// ============================================================
//  ESTRUCTURAS
// ============================================================

struct ConfigLiga {
    string nombre;
    int puntosVictoria;
    int puntosEmpate;
    int puntosDerrota;
    vector<string> equipos;
};

struct Partido {
    string fecha;
    string local;
    string visitante;
    int golesLocal;
    int golesVisitante;
};

struct Equipo {
    string nombre;
    int jugados;
    int ganados;
    int empatados;
    int perdidos;
    int golesFavor;
    int golesContra;
    int diferencia;
    int puntos;
};

// ============================================================
//  FUNCIONES DE ARCHIVOS - LECTURA
// ============================================================

bool leerConfig(const string& ruta, ConfigLiga& cfg) {
    ifstream archivo(ruta.c_str());
    if (!archivo.is_open()) {
        cout << "ERROR: No se puede abrir " << ruta << endl;
        return false;
    }

    bool tieneLiga     = false;
    bool tieneVictoria = false;
    bool tieneEmpate   = false;
    bool tieneDerrota  = false;

    string linea;
    while (getline(archivo, linea)) {
        if (linea.empty() || linea[0] == '#') continue;

        int pos = -1;
        for (int i = 0; i < (int)linea.size(); i++) {
            if (linea[i] == '=') { pos = i; break; }
        }
        if (pos == -1) continue;

        string clave = linea.substr(0, pos);
        string valor = linea.substr(pos + 1);

        if      (clave == "liga")     { cfg.nombre         = valor; tieneLiga     = true; }
        else if (clave == "victoria") { cfg.puntosVictoria = atoi(valor.c_str()); tieneVictoria = true; }
        else if (clave == "empate")   { cfg.puntosEmpate   = atoi(valor.c_str()); tieneEmpate   = true; }
        else if (clave == "derrota")  { cfg.puntosDerrota  = atoi(valor.c_str()); tieneDerrota  = true; }
        else if (clave == "equipo")   { cfg.equipos.push_back(valor); }
    }
    archivo.close();

    if (!tieneLiga || !tieneVictoria || !tieneEmpate || !tieneDerrota) {
        cout << "ERROR: config.txt tiene formato invalido. Faltan campos obligatorios." << endl;
        return false;
    }
    if (cfg.equipos.empty()) {
        cout << "ERROR: config.txt no tiene ningun equipo." << endl;
        return false;
    }
    return true;
}

bool leerPartidos(const string& ruta, vector<Partido>& partidos) {
    ifstream archivo(ruta.c_str());
    if (!archivo.is_open()) {
        return true; // No es error fatal: puede no haber partidos aun
    }

    string linea;
    while (getline(archivo, linea)) {
        if (linea.empty()) continue;

        // Formato: fecha;local;visitante;golesLocal;golesVisitante
        stringstream ss(linea);
        string fecha, local, visitante, gl, gv;

        if (!getline(ss, fecha,     ';')) continue;
        if (!getline(ss, local,     ';')) continue;
        if (!getline(ss, visitante, ';')) continue;
        if (!getline(ss, gl,        ';')) continue;
        if (!getline(ss, gv,        ';')) continue;

        Partido p;
        p.fecha          = fecha;
        p.local          = local;
        p.visitante      = visitante;
        p.golesLocal     = atoi(gl.c_str());
        p.golesVisitante = atoi(gv.c_str());
        partidos.push_back(p);
    }
    archivo.close();
    return true;
}

void leerYMostrarJornadas(const string& ruta) {
    ifstream archivo(ruta.c_str());
    if (!archivo.is_open()) {
        cout << "No hay jornadas registradas aun." << endl;
        return;
    }

    string linea;
    while (getline(archivo, linea)) {
        if (linea.empty()) continue;
        if (linea.size() >= 8 && linea.substr(0, 8) == "JORNADA=") {
            cout << "\n--- Jornada " << linea.substr(8) << " ---" << endl;
        } else if (linea == "FIN_JORNADA") {
            // Bloque terminado
        } else {
            cout << "  " << linea << endl;
        }
    }
    archivo.close();
}

// ============================================================
//  FUNCIONES DE ARCHIVOS - ESCRITURA
// ============================================================

bool guardarPartido(const string& ruta, const Partido& p) {
    ofstream archivo(ruta.c_str(), ios::app);
    if (!archivo.is_open()) {
        cout << "ERROR: No se puede escribir en " << ruta << endl;
        return false;
    }
    archivo << p.fecha << ";"
            << p.local << ";"
            << p.visitante << ";"
            << p.golesLocal << ";"
            << p.golesVisitante << endl;
    archivo.close();
    return true;
}

bool guardarJornada(const string& ruta, int numero, const string& fecha,
                    const vector<Partido>& partidos) {
    ofstream archivo(ruta.c_str(), ios::app);
    if (!archivo.is_open()) {
        cout << "ERROR: No se puede escribir en " << ruta << endl;
        return false;
    }
    archivo << "JORNADA=" << numero << endl;
    archivo << "FECHA="   << fecha  << endl;
    for (int i = 0; i < (int)partidos.size(); i++) {
        archivo << partidos[i].local << " "
                << partidos[i].golesLocal << " - "
                << partidos[i].golesVisitante << " "
                << partidos[i].visitante << endl;
    }
    archivo << "FIN_JORNADA" << endl;
    archivo.close();
    return true;
}

bool guardarTabla(const string& ruta, const vector<Equipo>& tabla) {
    ofstream archivo(ruta.c_str());
    if (!archivo.is_open()) {
        cout << "ERROR: No se puede escribir en " << ruta << endl;
        return false;
    }
    archivo << "#   Equipo                   PJ  PG  PE  PP  GF  GC   DG  PTS" << endl;
    archivo << string(62, '-') << endl;
    for (int i = 0; i < (int)tabla.size(); i++) {
        const Equipo& e = tabla[i];
        string nombre = e.nombre;
        while ((int)nombre.size() < 24) nombre += " ";
        archivo << (i + 1) << "   " << nombre
                << " " << e.jugados
                << "   " << e.ganados
                << "   " << e.empatados
                << "   " << e.perdidos
                << "   " << e.golesFavor
                << "   " << e.golesContra;
        if (e.diferencia >= 0) archivo << "  +" << e.diferencia;
        else                   archivo << "  "  << e.diferencia;
        archivo << "   " << e.puntos << endl;
    }
    archivo.close();
    return true;
}

// ============================================================
//  FUNCIONES DE LOGICA
// ============================================================

// Uso obligatorio de punteros: modifica el Equipo original directamente
void actualizarEstadisticas(Equipo* e, int golesFavor, int golesContra,
                             int pVictoria, int pEmpate, int pDerrota) {
    e->jugados++;
    e->golesFavor  += golesFavor;
    e->golesContra += golesContra;
    e->diferencia   = e->golesFavor - e->golesContra;

    if (golesFavor > golesContra) {
        e->ganados++;
        e->puntos += pVictoria;
    } else if (golesFavor == golesContra) {
        e->empatados++;
        e->puntos += pEmpate;
    } else {
        e->perdidos++;
        e->puntos += pDerrota;
    }
}

vector<Equipo> construirTabla(const vector<Partido>& partidos, const ConfigLiga& cfg) {
    vector<Equipo> tabla;

    for (int i = 0; i < (int)cfg.equipos.size(); i++) {
        Equipo e;
        e.nombre      = cfg.equipos[i];
        e.jugados     = 0;
        e.ganados     = 0;
        e.empatados   = 0;
        e.perdidos    = 0;
        e.golesFavor  = 0;
        e.golesContra = 0;
        e.diferencia  = 0;
        e.puntos      = 0;
        tabla.push_back(e);
    }

    for (int i = 0; i < (int)partidos.size(); i++) {
        const Partido& p = partidos[i];

        for (int j = 0; j < (int)tabla.size(); j++) {
            if (tabla[j].nombre == p.local) {
                actualizarEstadisticas(&tabla[j], p.golesLocal, p.golesVisitante,
                                       cfg.puntosVictoria, cfg.puntosEmpate, cfg.puntosDerrota);
                break;
            }
        }
        for (int j = 0; j < (int)tabla.size(); j++) {
            if (tabla[j].nombre == p.visitante) {
                actualizarEstadisticas(&tabla[j], p.golesVisitante, p.golesLocal,
                                       cfg.puntosVictoria, cfg.puntosEmpate, cfg.puntosDerrota);
                break;
            }
        }
    }
    return tabla;
}

bool compararEquipos(const Equipo& a, const Equipo& b) {
    if (a.puntos     != b.puntos)     return a.puntos     > b.puntos;
    if (a.diferencia != b.diferencia) return a.diferencia > b.diferencia;
    return a.golesFavor > b.golesFavor;
}

void ordenarTabla(vector<Equipo>& tabla) {
    sort(tabla.begin(), tabla.end(), compararEquipos);
}

void mostrarTablaConsola(const vector<Equipo>& tabla) {
    cout << endl;
    cout << "#   Equipo                   PJ  PG  PE  PP  GF  GC   DG  PTS" << endl;
    cout << string(62, '-') << endl;

    for (int i = 0; i < (int)tabla.size(); i++) {
        const Equipo& e = tabla[i];

        if (i + 1 < 10) cout << " ";
        cout << (i + 1) << "  ";

        string nombre = e.nombre;
        cout << nombre;
        for (int s = (int)nombre.size(); s < 24; s++) cout << " ";

        cout << " " << e.jugados
             << "   " << e.ganados
             << "   " << e.empatados
             << "   " << e.perdidos
             << "   " << e.golesFavor
             << "   " << e.golesContra;

        if (e.diferencia >= 0) cout << "  +" << e.diferencia;
        else                   cout << "  "  << e.diferencia;

        cout << "   " << e.puntos << endl;
    }
    cout << endl;
}

// ============================================================
//  FUNCIONES DE INTERFAZ
// ============================================================

int mostrarMenu(const string& nombreLiga) {
    cout << "\n========================================" << endl;
    cout << "       " << nombreLiga                     << endl;
    cout << "========================================" << endl;
    cout << "1. Ver tabla de posiciones"               << endl;
    cout << "2. Registrar resultado de un partido"     << endl;
    cout << "3. Ver historial de jornadas"             << endl;
    cout << "4. Ver todos los partidos jugados"        << endl;
    cout << "5. Salir"                                 << endl;
    cout << "Elige una opcion: ";

    int opcion;
    cin >> opcion;
    cin.ignore(1000, '\n');
    return opcion;
}

bool ingresarPartido(const ConfigLiga& cfg, Partido& p) {
    cout << "\n--- Registrar nuevo partido ---" << endl;
    cout << "Equipos disponibles:" << endl;
    for (int i = 0; i < (int)cfg.equipos.size(); i++) {
        cout << "  " << (i + 1) << ". " << cfg.equipos[i] << endl;
    }

    int idxLocal, idxVisitante;

    cout << "Selecciona equipo LOCAL (numero): ";
    cin >> idxLocal;
    cin.ignore(1000, '\n');
    idxLocal--;

    if (idxLocal < 0 || idxLocal >= (int)cfg.equipos.size()) {
        cout << "Opcion invalida." << endl;
        return false;
    }

    cout << "Selecciona equipo VISITANTE (numero): ";
    cin >> idxVisitante;
    cin.ignore(1000, '\n');
    idxVisitante--;

    if (idxVisitante < 0 || idxVisitante >= (int)cfg.equipos.size()) {
        cout << "Opcion invalida." << endl;
        return false;
    }
    if (idxLocal == idxVisitante) {
        cout << "ERROR: Los dos equipos deben ser distintos." << endl;
        return false;
    }

    cout << "Goles del equipo LOCAL (" << cfg.equipos[idxLocal] << "): ";
    cin >> p.golesLocal;
    cin.ignore(1000, '\n');

    cout << "Goles del equipo VISITANTE (" << cfg.equipos[idxVisitante] << "): ";
    cin >> p.golesVisitante;
    cin.ignore(1000, '\n');

    if (p.golesLocal < 0 || p.golesVisitante < 0) {
        cout << "ERROR: Los goles no pueden ser negativos." << endl;
        return false;
    }

    time_t t = time(0);
    char buf[20];
    strftime(buf, sizeof(buf), "%Y-%m-%d", localtime(&t));
    p.fecha     = string(buf);
    p.local     = cfg.equipos[idxLocal];
    p.visitante = cfg.equipos[idxVisitante];

    cout << "Partido registrado: "
         << p.local << " " << p.golesLocal
         << " - "   << p.golesVisitante
         << " "     << p.visitante << endl;
    return true;
}

int obtenerNumeroJornada(const string& ruta) {
    ifstream archivo(ruta.c_str());
    if (!archivo.is_open()) return 1;

    int count = 0;
    string linea;
    while (getline(archivo, linea)) {
        if (linea.size() >= 8 && linea.substr(0, 8) == "JORNADA=") count++;
    }
    archivo.close();
    return count + 1;
}

// ============================================================
//  MAIN
// ============================================================

int main() {
    ConfigLiga cfg;
    if (!leerConfig("config.txt", cfg)) {
        return 1;
    }

    vector<Partido> partidosSesion;

    int opcion = 0;
    while (opcion != 5) {
        opcion = mostrarMenu(cfg.nombre);

        if (opcion == 1) {
            vector<Partido> todos;
            if (!leerPartidos("partidos.txt", todos)) {
                cout << "Error leyendo partidos." << endl;
            } else {
                vector<Equipo> tabla = construirTabla(todos, cfg);
                ordenarTabla(tabla);
                mostrarTablaConsola(tabla);

                cout << "Deseas guardar la tabla en tabla.txt? (1=Si / 2=No): ";
                int resp;
                cin >> resp;
                cin.ignore(1000, '\n');
                if (resp == 1) {
                    if (guardarTabla("tabla.txt", tabla))
                        cout << "Tabla guardada en tabla.txt" << endl;
                }
            }

        } else if (opcion == 2) {
            Partido p;
            if (ingresarPartido(cfg, p)) {
                if (guardarPartido("partidos.txt", p)) {
                    partidosSesion.push_back(p);
                    cout << "Partido guardado correctamente." << endl;
                }
            }

        } else if (opcion == 3) {
            leerYMostrarJornadas("fechas.txt");

        } else if (opcion == 4) {
            vector<Partido> todos;
            if (!leerPartidos("partidos.txt", todos)) {
                cout << "Error leyendo partidos." << endl;
            } else if (todos.empty()) {
                cout << "No hay partidos registrados aun." << endl;
            } else {
                cout << "\n--- Todos los partidos ---" << endl;
                for (int i = 0; i < (int)todos.size(); i++) {
                    const Partido& p = todos[i];
                    cout << p.fecha << "  "
                         << p.local << " "
                         << p.golesLocal << " - "
                         << p.golesVisitante << " "
                         << p.visitante << endl;
                }
            }

        } else if (opcion == 5) {
            if (!partidosSesion.empty()) {
                int numJornada = obtenerNumeroJornada("fechas.txt");
                time_t t = time(0);
                char buf[20];
                strftime(buf, sizeof(buf), "%Y-%m-%d", localtime(&t));
                string fechaHoy = string(buf);
                guardarJornada("fechas.txt", numJornada, fechaHoy, partidosSesion);
                cout << "Jornada " << numJornada << " guardada." << endl;
            }
            cout << "Hasta luego!" << endl;

        } else {
            cout << "Opcion no valida. Intenta de nuevo." << endl;
        }
    }

    return 0;
}
