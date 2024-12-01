/*
 * MAPA - LINGUAGEM E TÉCNICAS DE PROGRAMAÇÃO - 54_2024
 * Aluno: Vinicius Caetano da Silva
 * Data: 04/11/2024
 */


#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <stdbool.h>
#include <stdlib.h>


#define VERSION "1.0.0"

#define COMPANY_NAME "RESTAURANTE LAVOISIER" // Nome do restaurante
#define MAX_CAPACITY_PER_DAY 200     // Capacidade máxima de pessoas por dia
#define DAYS_OF_WEEK 4               // Quinta, Sexta, Sábado, Domingo (1-4)
#define DB_FILE "reservations.data" // Arquivo de banco de dados


// Tipos de mensagens
enum LogLevel {
    SUCCESS = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3
};

// Estrutura de dados para armazenar uma reserva
struct Reservation {
    char name[50];
    char cpf[15];
    int day;
    int people_count;
};

// Array para armazenar as reservas
struct Reservation reservations[MAX_CAPACITY_PER_DAY * DAYS_OF_WEEK];
int total_reservations = 0; // Total de reservas
int daily_people_count[DAYS_OF_WEEK] = {0}; // Contagem de pessoas por dia (1-Quinta, 2-Sexta, 3-Sábado, 4-Domingo)
int daily_reservations_count[DAYS_OF_WEEK] = {0}; // Total de reservas por dia
const char *week_days[] = {"Quinta-feira", "Sexta-feira", "Sábado", "Domingo"}; // Nomes dos dias da semana

/**
 * Função para limpar a tela do console.
 */
void clear_screen() {
    printf("\nPressione qualquer tecla para continuar...");
    getchar();  // Aguarda o usuário pressionar qualquer tecla
    #ifdef _WIN32
            system("cls");
    #elif __linux__ || __APPLE__
        system("clear");
    #else
            printf("Sistema Operacional não suportado.\n");
    #endif
}

/**
 * Função para limpar o buffer do teclado.
 */
void flush_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}


/**
 * Função para ler uma string informada pelo usuário.
 * @param str String a ser lida.
 * @param size Tamanho máximo da string.
 * @return true se a string foi lida com sucesso, false caso contrário.
 */
bool read_string(char *str, size_t size) {
    if (fgets(str, size, stdin) == NULL) {
        return false;
    }
    str[strcspn(str, "\n")] = 0;
    return true;
}

/**
 * Função para ler um inteiro informado pelo usuário.
 * @param value Valor a ser lido.
 * @return true se o valor foi lido com sucesso, false caso contrário.
 */
bool read_int(int *value) {
    if (scanf("%d", value) == EOF) return false;
    while (getchar() != '\n') {}
    return true;
}


/**
 * Função para construir o menu principal.
 */
void build_menu() {
    printf("\n=== %s - Gestão de reservas v%s ===\n", COMPANY_NAME, VERSION);
    printf("1 - Fazer Reserva\n");
    printf("2 - Listar Reservas\n");
    printf("3 - Cancelar Reserva\n");
    printf("4 - Total de Pessoas por Dia\n");
    printf("5 - Sair\n");
    printf("Escolha uma opção: ");
}

/**
 * Função para exibir mensagens ao usuário.
 * @param msg Mensagem a ser exibida.
 * @param level Nível da mensagem (INFO, WARNING, ERROR).
 */
void display_message(const char *msg, const int level) {
    switch (level) {
        case SUCCESS:
            printf("\033[0;32m[SUCESSO] %s \033[0m\n", msg);
            break;
        case INFO:
            printf("\033[0;34m[INFO] %s \033[0m\n", msg);
            break;
        case WARNING:
            printf("\033[0;33m[ATENÇÃO] %s \033[0m\n", msg);
            break;
        case ERROR:
            printf("\033[0;31m[ERRO] %s \033[0m\n", msg);
            break;
        default:
            printf("\033[0;32m[INFO] %s \033[0m\n", msg);
            break;
    }
}

/**
 * Função para salvar as reservas no arquivo.
 */
void save_reservations_to_file() {
    FILE *file = fopen(DB_FILE, "w");
    if (!file) {
        display_message("Erro ao abrir o arquivo para salvar as reservas.", ERROR);
        return;
    }

    for (int i = 0; i < total_reservations; i++) {
        struct Reservation *res = &reservations[i];
        fprintf(file, "%s\n%s\n%d\n%d\n", res->name, res->cpf, res->day, res->people_count);
    }

    fclose(file);
}

/**
 * Função para carregar as reservas do arquivo.
 */
void load_reservations_from_file() {
    display_message("Carregando as reservas...", INFO);
    FILE *file = fopen(DB_FILE, "r");
    if (!file) {
        display_message("Nenhuma reserva anterior encontrada.", INFO);
        return;
    }

    struct Reservation res;
    while (fscanf(file, "%49[^\n]\n%14[^\n]\n%d\n%d\n", res.name, res.cpf, &res.day, &res.people_count) == 4) {
        reservations[total_reservations++] = res;
        daily_people_count[res.day - 1] += res.people_count;
        daily_reservations_count[res.day - 1]++;
    }

    fclose(file);
    display_message("Reservas carregadas com sucesso.", SUCCESS);
}


bool is_valid_count_people(int people_count) {
    if (people_count < 1 || people_count > 200) {
        display_message("Quantidade de pessoas inválida. Informe um valor entre 1 e 200.", ERROR);
        return false;
    }
    return true;
}

/**
 * Função para validar o nome.
 * O nome deve ter entre 3 e 50 caracteres e ser composto apenas por letras e espaços.
 * @param name Nome a ser validado.
 * @return true se o nome for válido, false caso contrário.
 */
bool is_valid_name(const char name[]) {
    if (strlen(name) < 3) {
        display_message("Nome deve conter pelo menos 3 caracteres. Tente novamente.", ERROR);
        return false;
    }
    if (strlen(name) > 50) {
        display_message("Nome deve conter no máximo 50 caracteres. Tente novamente.", ERROR);
        return false;
    }
    for (int i = 0; i < strlen(name); i++) {
        if (!isalpha(name[i]) && name[i] != ' ') {
            display_message("Nome deve conter apenas letras e espaços. Tente novamente.", ERROR);
            return false;
        }
    }
    return true;
}

/**
 * Função para validar o CPF.
 * O CPF deve conter exatamente 11 dígitos numéricos.
 * @param cpf CPF a ser validado.
 * @return true se o CPF for válido, false caso contrário.
 */
bool is_valid_cpf(const char cpf[]) {
    if (strlen(cpf) != 11) {
        display_message("CPF deve conter 11 dígitos. Tente novamente.", ERROR);
        return false;
    }

    for (int i = 0; i < 11; i++) {
        if (!isdigit(cpf[i])) {
            display_message("CPF deve conter apenas números. Tente novamente.", ERROR);
            return false;
        }
    }

    // Validação do CPF
    int digito1 = 0, digito2 = 0;
    for (int i = 0, j = 10; i < 9; i++, j--) {
        digito1 += (cpf[i] - '0') * j;
    }
    digito1 = digito1 % 11 < 2 ? 0 : 11 - digito1 % 11;

    for (int i = 0, j = 11; i < 10; i++, j--) {
        digito2 += (cpf[i] - '0') * j;
    }
    digito2 = digito2 % 11 < 2 ? 0 : 11 - digito2 % 11;

    if (digito1 != cpf[9] - '0' || digito2 != cpf[10] - '0') {
        display_message("CPF digitado é inválido. Tente novamente.", ERROR);
        return false;
    }

    return true;
}

/**
 * Função para verificar se um dia da semana está cheio.
 * @param day Dia da semana (1-Quinta, 2-Sexta, 3-Sábado, 4-Domingo).
 * @return true se o dia estiver cheio, false caso contrário.
 */
bool is_day_full(int day) {
    return daily_people_count[day - 1] >= MAX_CAPACITY_PER_DAY;
}

/**
 * Função para verificar se é possível acomodar um grupo de pessoas em um dia específico.
 * @param day Dia da semana (1-Quinta, 2-Sexta, 3-Sábado, 4-Domingo).
 * @param people_count Número de pessoas para a reserva.
 * @return true se for possível acomodar, false caso contrário.
 */
bool can_accommodate(int day, int people_count) {
    return (daily_people_count[day - 1] + people_count) <= MAX_CAPACITY_PER_DAY;
}

/**
 * Função para inserir uma nova reserva.
 */
void new_reservation() {
    int day, people_count;

    // Solicitar o dia da reserva
    do {
        display_message("Para cancelar a reserva, digite '0'", INFO);
        printf("Dia da Reserva (1-Quinta, 2-Sexta, 3-Sábado, 4-Domingo): ");
        if (!read_int(&day) || day < 1 || day > 4) {
            display_message("Nova reserva abortada.", INFO);
            return;
        }
    } while (day < 1 || day > 4);

    // Verificar se o dia está cheio
    if (is_day_full(day)) {
        display_message("Não há mais mesas disponíveis para este dia.", ERROR);
        return;
    }

    // Solicitar a quantidade de pessoas
    printf("Quantidade de Pessoas: ");
    if (!read_int(&people_count) || !is_valid_count_people(people_count)) {
        display_message("Nova reserva abortada.", INFO);
        return;
    }


    // Verificar se a quantidade de pessoas cabe no dia
    if (!can_accommodate(day, people_count)) {
        display_message("Não há mesas suficientes para essa quantidade de pessoas neste dia.", ERROR);
        return;
    }

    // Criar e validar a reserva
    struct Reservation new_reservation;
    do {
        printf("Nome: ");
        if (!read_string(new_reservation.name, sizeof(new_reservation.name)) || !is_valid_name(new_reservation.name)) {
            continue;
        }
        break;
    } while (true);

    do {
        printf("CPF (apenas números): ");
        if (!read_string(new_reservation.cpf, sizeof(new_reservation.cpf)) || !is_valid_cpf(new_reservation.cpf)) {
            continue;
        }
        break;
    } while (true);

    new_reservation.day = day;
    new_reservation.people_count = people_count;

    // Adicionar a nova reserva à lista
    reservations[total_reservations++] = new_reservation;
    daily_people_count[day - 1] += people_count;
    daily_reservations_count[day - 1]++;

    for (int i = 0; new_reservation.name[i]; i++) {
        new_reservation.name[i] = (char)toupper(new_reservation.name[i]);
    }


    char buffer[100];
    snprintf(buffer, sizeof(buffer), "Reserva #%d em nome de %s realizada com sucesso.", total_reservations, new_reservation.name);
    display_message(buffer, SUCCESS);
    save_reservations_to_file();

}

/**
 * Função para listar todas as reservas.
 */
void list_reservations() {
    if (total_reservations == 0) {
        display_message("Nenhuma reserva realizada.", INFO);
        return;
    }

    for (int i = 0; i < total_reservations; i++) {
        struct Reservation *res = &reservations[i];
        printf("\n========== Reserva #%d ==========\n", i + 1);
        printf("Nome: %s\n", res->name);
        printf("CPF: %s\n", res->cpf);
        printf("Dia: %s\n", week_days[res->day - 1]);
        printf("Número de Pessoas: %d\n", res->people_count);
        printf("\n");
    }
}

/**
 * Função para gerar um relatório com as informações de reservas.
 */
void generate_report() {
    printf("\n=== Relatório Gerencial ===\n");

    for (int i = 0; i < DAYS_OF_WEEK; i++) {
        int capacity_used = daily_people_count[i];
        int total_reservations_today = daily_reservations_count[i];
        double percentage_used = (capacity_used / (double)MAX_CAPACITY_PER_DAY) * 100;
        printf("\n%s:\n", week_days[i]);
        printf("  Total de reservas: %d\n", total_reservations_today);
        printf("  Total de pessoas: %d\n", capacity_used);
        printf("  %d/%d capacidade ocupada (%.2f%%)\n",
               capacity_used, MAX_CAPACITY_PER_DAY,
               percentage_used);
    }
}

/**
 * Função para cancelar uma reserva.
 */
void cancel_reservation() {
    list_reservations();

    int reservation_num;
    printf("Digite o número da reserva que deseja cancelar (1-%d): ", total_reservations);

    if (!read_int(&reservation_num) || reservation_num < 1 || reservation_num > total_reservations) {
        display_message("Número de reserva inválido. Retornando ao menu principal...", ERROR);
        return;
    }

    const struct Reservation *res = &reservations[--reservation_num];

    // Ajustar as contagens após o cancelamento
    daily_people_count[res->day - 1] -= res->people_count;
    daily_reservations_count[res->day - 1]--;

    // Reorganizar o array de reservas
    for (int i = reservation_num; i < total_reservations - 1; i++) {
        reservations[i] = reservations[i + 1];
    }

    total_reservations--;

    char buffer[100];
    snprintf(buffer, sizeof(buffer), "Reserva #%d cancelada com sucesso.", reservation_num + 1);
    display_message(buffer, SUCCESS);
    save_reservations_to_file();

}

int main() {
    setlocale(LC_ALL, "Portuguese");
    load_reservations_from_file();

    int option;

    do {
        build_menu();
        if (!read_int(&option)) {
            display_message("Opção inválida. Tente novamente.", WARNING);
            continue;
        }

        switch (option) {
            case 1:
                new_reservation();
                break;
            case 2:
                list_reservations();
                break;
            case 3:
                cancel_reservation();
                break;
            case 4:
                generate_report();
                break;
            case 5:
                display_message("Encerrando o programa...", INFO);
                break;
            default:
                display_message("Opção inválida. Tente novamente.", WARNING);
        }

        if (option != 5) {
            clear_screen();
        }
    } while (option != 5);

    return 0;
}
