// -----------------------------------main.c -------------------------------------
#include "uart.h"

#define MAX_CMD_SIZE 100

//-------------------------------------Some Custom function------------------------------------------------------
int cus_strcmp(const char *s1, const char *s2) // just a string copy : D
{
    while (*s1 && *s2 && *s1 == *s2)
    {
        ++s1;
        ++s2;
    }
    return *s1 - *s2;
}

char *cus_strtok(char *str, const char *delim)
{
    static char *p = (char *)0;
    if (*str != '\0')
        p = str;

    if (p == (char *)0 || *p == '\0')
        return (char *)0;

    char *start = p;

    while (*p != '\0')
    {
        const char *d = delim;
        while (*d != '\0')
        {
            if (*p == *d)
            {
                *p = '\0';
                ++p;
                if (*p == '\0')
                    p = (char *)0;
                return start;
            }
            ++d;
        }
        ++p;
    }

    return start;
}

//------------------main function-----------------------------------------------------
void errors()
{
    uart_puts("\nInvalid Commands");
    uart_puts("\n");
}

char *currentColors = "\x1b[37m";
char *currentBackgroundColors = "\x1b[40m";
void setBackgroundColor(char *backgroundColors)
{
    if (cus_strcmp(backgroundColors, "red") == 0)
    {
        currentBackgroundColors = "\x1b[41m"; // 41 is red
    }
    else if (cus_strcmp(backgroundColors, "green") == 0)
    {
        currentBackgroundColors = "\x1b[42m"; // 42 is green
    }
    else if (cus_strcmp(backgroundColors, "yellow") == 0)
    {
        currentBackgroundColors = "\x1b[43m"; // 43 is yellow
    }
    else if (cus_strcmp(backgroundColors, "blue") == 0)
    {
        currentBackgroundColors = "\x1b[44m"; // 44 is blue
    }
    else if (cus_strcmp(backgroundColors, "purple") == 0)
    {
        currentBackgroundColors = "\x1b[45m"; // 45 is purple
    }
    else if (cus_strcmp(backgroundColors, "cyan") == 0)
    {
        currentBackgroundColors = "\x1b[46m"; // 46 is cyan
    }
    else if (cus_strcmp(backgroundColors, "white") == 0)
    {
        currentBackgroundColors = "\x1b[47m"; // 47 is white
    }
    else
    {
        uart_puts("Invalid background colors! Please use red, green, yellow, blue, purple, cyan, or white.\n");
    }
}

void setColor(char *colors)
{
    if (cus_strcmp(colors, "red") == 0)
    {

        currentColors = "\x1b[31m";
    }
    else if (cus_strcmp(colors, "green") == 0)
    {
        currentColors = "\x1b[32m";
    }
    else if (cus_strcmp(colors, "yellow") == 0)
    {
        currentColors = "\x1b[33m";
    }
    else if (cus_strcmp(colors, "blue") == 0)
    {
        currentColors = "\x1b[34m";
    }
    else if (cus_strcmp(colors, "purple") == 0)
    {
        currentColors = "\x1b[35m";
    }
    else if (cus_strcmp(colors, "cyan") == 0)
    {
        currentColors = "\x1b[36m";
    }
    else if (cus_strcmp(colors, "white") == 0)
    {
        currentColors = "\x1b[37m";
    }
    else
    {
        uart_puts("Invalid colors! Please use red, green, yellow, blue, purple, cyan, or white.\n");
    }
}
void cli()
{
    static char cli_buffer[MAX_CMD_SIZE];
    static int index = 0;
    // read and send back each char
    char c = uart_getc();
    uart_sendc(c);

    // put into a buffer until got new line or get backspace character
    if (c != '\n' && c != '\b')
    {
        cli_buffer[index] = c; // Store into the buffer
        index++;
    }

    // if user typein backspace it will delete the output and shift the index --
    if (c == '\b')
    {

        if (index <= 0)
        {
            uart_puts(" ");
            return;
        }
        uart_puts(" \b");
        index--;
        cli_buffer[index] = '\0';
    }

    else if (c == '\n')
    {
        cli_buffer[index] = '\0';
        char *token = cus_strtok(cli_buffer, " ");
        uart_puts("\nGot commands: ");
        uart_puts(cli_buffer);
        uart_puts("\n");
        /* Compare with supported commands and execute
         * ........................................... */
        // HELP FUNCTION
        if (cus_strcmp(token, "help") == 0)
        {
            char *secondToken = cus_strtok("\0", " ");
            if (secondToken != 0)
            {

                if (cus_strcmp(secondToken, "help") == 0)
                {
                    uart_puts("\nhelp:                     - Show brief information of all commands");
                    uart_puts("\nhelp <command_name>       - Show full information of the command");
                }
                else if (cus_strcmp(secondToken, "clear") == 0)
                {
                    uart_puts("\nclear                     - Clear screen (in our terminal it will scroll down to currentposition of the cursor).");
                }
                else if (cus_strcmp(secondToken, "setcolor") == 0)
                {
                    uart_puts("\nsetcolor                  - Set text color, and/or background color of the console to one of the following colors: BLACK, RED, GREEN, YELLOW, BLUE, PURPLE, CYAN, WHITE");
                    uart_puts("\n-t <text color> \n-b <background color>");
                }
                else if (cus_strcmp(secondToken, "showinfo") == 0)
                {
                    uart_puts("\nshowinfo                  - Show board revision and board MAC address in correct format/ meaningful information");
                }
            }

            else
            {
                uart_puts("\nhelp");
                uart_puts("\nclear");
                uart_puts("\nsetcolor");
                uart_puts("\nshowinfo");
            }
        }

        if (cus_strcmp(token, "setcolor") == 0)
        {
            char *secondToken = cus_strtok("\0", " ");
            if (cus_strcmp(secondToken, "-t") == 0)
            {
                char *colors = cus_strtok("\0", " "); // Extract color name
                setColor(colors);
                char *thirdToken = cus_strtok("\0", " ");
                if (cus_strcmp(thirdToken, "-b") == 0)
                {
                    char *backgroundColors = cus_strtok("\0", " "); // Extract color name
                    setBackgroundColor(backgroundColors);
                }
                else
                {
                    errors();
                }
            }

            else if (cus_strcmp(secondToken, "-b") == 0)
            {
                char *backgroundColors = cus_strtok("\0", " "); // Extract color name
                setBackgroundColor(backgroundColors);
                char *thirdToken = cus_strtok("\0", " ");
                if (cus_strcmp(thirdToken, "-t") == 0)
                {
                    char *colors = cus_strtok("\0", " "); // Extract color name
                    setColor(colors);
                }
                else
                {
                    errors();
                }
            }
            else
            {
                errors();
            }
        }
        // CLEAR FUNCTION
        else if (cus_strcmp(token, "clear") == 0)
        {
            uart_puts("\e[1;1H\e[2J");
        }

        else if (cus_strcmp(token, "clear") != 0 && cus_strcmp(token, "setcolor") == 0 && cus_strcmp(token, "help") == 0)
        {
            errors();
        }
        uart_puts(currentColors);
        uart_puts(currentBackgroundColors);
        uart_puts("\nNingOS:> ");
        index = 0;
    }
}

//------------------------------------------------------main--------------------------------------
int main()
{
    // intitialize UART

    uart_init();
    // say hello

    uart_puts(" ______ ______ ______ _______ ___  _  _    ___   ___ \n"
              "|  ____|  ____|  ____|__   __|__ \\| || |  / _ \\ / _ \\ \n"
              "| |__  | |__  | |__     | |     ) | || |_| | | | (_) | \n"
              "|  __| |  __| |  __|    | |    / /|__   _| | | |\\__, | \n"
              "| |____| |____| |____   | |   / /_   | | | |_| |  / / \n"
              "|______|______|______|  |_|  |____|  |_|  \\___/  /_/ \n"
              "\n"
              "\n"
              "\n"
              " ____          _____  ______       ____   ______ \n"
              "|  _ \\   /\\   |  __ \\|  ____|     / __ \\ / ____| \n"
              "| |_) | /  \\  | |__) | |__       | |  | | (___ \n"
              "|  _ < / /\\ \\ |  _  /|  __|      | |  | |\\___ \\ \n"
              "| |_) / ____ \\| | \\ \\| |____     | |__| |____) | \n"
              "|____/_/    \\_\\_|  \\_\\______|     \\____/|_____/ \n"
              "\n"
              "Developed by Tran Thanh Tu - S3957386\n");
    uart_puts("NingOS:> ");

    // echo everything back
    while (1)
    {
        // read each char

        cli();
    }
}