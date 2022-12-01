
#include <Arduino.h>


void setup_wifi (void);

void HTML_header (String& a_string, const char* page_title);

void handle_DocumentRoot ();

void handle_NotFound (void);

void handle_Toggle_LED (void);

void handle_CSV (void);

void task_webserver (void* p_params);