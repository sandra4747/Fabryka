# Symulacja Fabryki

## Opis projektu
Projekt symuluje działanie fabryki z trzema dostawcami, dwoma stanowiskami produkcyjnymi oraz dyrektorem zarządzającym. Fabryka wykorzystuje **pamięć dzieloną**, **procesy** i **semafory** do synchronizacji pracy.

## Funkcje

### Procesy:
- **Dyrektor**: Wydaje polecenia zarządzające.
- **Dostawcy (X, Y, Z)**: Dostarczają komponenty.
- **Monterzy (A, B)**: Tworzą produkty z komponentów.

### Polecenia dyrektora:
1. **Zakończenie pracy magazynu**: Polecenie zatrzymania operacji magazynowych.
2. **Zakończenie pracy fabryki**: Polecenie zamknięcia całej fabryki.
3. **Zapis stanu magazynu i zakończenie pracy**: Zapisuje aktualny stan magazynu i kończy działanie fabryki.
4. **Zakończenie pracy bez zapisywania stanu**: Kończy pracę fabryki bez zapisywania bieżącego stanu magazynu.

