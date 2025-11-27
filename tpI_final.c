//https://github.com/santi8547651717/TPI_Info1_santiago_gonzalez
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>
#include <time.h>

struct memory {
    char *response;
    size_t size;
};

static size_t cb(char *data, size_t size, size_t nmemb, void *clientp) {
    size_t realsize = nmemb;
    struct memory *mem = clientp;
    char *ptr = realloc(mem->response, mem->size + realsize + 1);

    if (!ptr) return 0;

    mem->response = ptr;
    memcpy(&(mem->response[mem->size]), data, realsize);
    mem->size += realsize;
    mem->response[mem->size] = 0;

    return realsize;
}

int main(void) {
    char *api_base = "https://api.telegram.org/bot%s/getUpdates?offset=%d";
    char token[50];
    char api_url[256];
    long long update_new = 0;
    long long update_id;

    FILE *f = fopen("token.txt", "r");
    if (f == NULL) {
        printf("No se pudo cargar su token!!");
        return 1;
    }
    fscanf(f, "%s", token);
    fclose(f);

    CURLcode res;
    CURL *curl = curl_easy_init();

    while (1) {
        // Tiempo
        time_t t;
        time(&t);  // obtiene el tiempo actual en segundos UNIX

        struct tm *tm_info = localtime(&t);
        char buffer[30];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);

        struct memory chunk = {0};
        snprintf(api_url, 256, "https://api.telegram.org/bot%s/getUpdates?offset=%lld", token, update_new);


        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, api_url);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

            res = curl_easy_perform(curl);
            if (res != 0) printf("Error Codigo: %d\n", res);

            printf("%s\n", chunk.response);

            // Buscar update_id
            char *act_id = strstr(chunk.response, "\"update_id\":");
            if (act_id != NULL) {
                act_id += strlen("\"update_id\":");
                update_id = atoll(act_id);
                printf("Update Id: %lld\n", update_id);
                update_new = update_id + 1;
            }

            // Buscar chat id
            long long chat_id = 0;
            char *chat_ptr = strstr(chunk.response, "\"chat\":{");
            if (chat_ptr != NULL) {
                chat_ptr = strstr(chat_ptr, "\"id\":");
                if (chat_ptr != NULL) {
                    chat_ptr += strlen("\"id\":");
                    chat_id = atoll(chat_ptr);
                    printf("Chat id: %lld\n", chat_id);
                }
            }

            // Buscar texto
            char *text_ptr = strstr(chunk.response, "\"text\":\"");
            if (text_ptr != NULL) {
                text_ptr += strlen("\"text\":\"");
                char *end = strchr(text_ptr, '"');
                if (end != NULL) {
                    *end = '\0';
                }
                printf("Texto recibido: %s\n", text_ptr);
            }

            // Buscar nombre usuario
            char *user = strstr(chunk.response, "\"first_name\":\"");
            char usuario[100] = "";
            if (user != NULL) {
                user += strlen("\"first_name\":\"");
                char *end = strchr(user, '"');
                if (end != NULL) {
                    *end = '\0';
                    strcpy(usuario, user);
                }
                printf("Usuario: %s\n", usuario);
            }

            //Guardar mensajes
            if (text_ptr!=0) {
                FILE *s= fopen("mensajes.txt", "a");
                if (s!=NULL) {
                    fprintf(s, "%s,%s:%s\n", buffer,usuario, text_ptr);
                    fclose(s);
                } else { printf("No se pudo abrir el archivo mensajes.txt!!");}

            // Enviar Mensaje
            char send_url[200];
            if (text_ptr != NULL && strcmp(text_ptr, "hola") == 0) {
                snprintf(send_url, 200,"https://api.telegram.org/bot%s/sendMessage?chat_id=%lld&text=Hola,%s",
                         token, chat_id, usuario);
                curl_easy_setopt(curl, CURLOPT_URL, send_url);
                curl_easy_perform(curl);
                FILE* s = fopen("mensajes.txt", "a");
                if (s != NULL) {
                    fprintf(s, "%s,     Bot:Hola,%s\n", buffer,usuario );
                    fclose(s);
                } else { printf("No se pudo abrir el archivo mensajes.txt!!");}
            } else if (text_ptr != NULL && strcmp(text_ptr, "chau") == 0) {
                snprintf(send_url, 200,
                         "https://api.telegram.org/bot%s/sendMessage?chat_id=%lld&text=Adios,%s",
                         token, chat_id, usuario);
                curl_easy_setopt(curl, CURLOPT_URL, send_url);
                curl_easy_perform(curl);
                FILE* s = fopen("mensajes.txt", "a");
                if (s != NULL) {
                    fprintf(s, "%s,     Bot:Adios,%s\n", buffer,usuario );
                    fclose(s);
                } else { printf("No se pudo abrir el archivo mensajes.txt!!");}
            } else if (text_ptr != NULL) {
                snprintf(send_url, 200,"https://api.telegram.org/bot%s/sendMessage?chat_id=%lld&text=%s%%20manda%%20algo%%20",token, chat_id, usuario);
                curl_easy_setopt(curl, CURLOPT_URL, send_url);
                curl_easy_perform(curl);
                FILE* s = fopen("mensajes.txt", "a");
                if (s != NULL) {
                    fprintf(s, "%s,     Bot:%s manda algo\n", buffer,usuario );
                    fclose(s);
                } else { printf("No se pudo abrir el archivo mensajes.txt!!");}

            }





                free(chunk.response);
                sleep(2);
            }
        }


    }
    return 0;
}
