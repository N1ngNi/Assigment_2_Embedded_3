// -----------------------------------main.c -------------------------------------
#include "uart.h"
#include "mbox.h"

#define MAX_CMD_SIZE 100
#define COMMANDS_SIZE 4
char *commands[] = {"help", "clear", "setcolor", "showinfo"};
char *history[100][100];
static int historyList = 0;
static int currentHistoryList = 0;
//-------------------------------------Some Custom function------------------------------------------------------
int cus_strcmp(const char *s1, const char *s2)
{
    while (*s1 && *s2 && *s1 == *s2)
    {
        ++s1;
        ++s2;
    }
    return *s1 - *s2;
}

int cus_strlen(const char *str)
{
    int length = 0;
    while (str[length] != '\0')
    {
        length++;
    }
    return length;
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

void cus_strcpy(char *dest, const char *src)
{
    while (*src)
    {
        *dest++ = *src++;
    }
    *dest = '\0';
}

//------------------main function-----------------------------------------------------
void errors()
{
    uart_puts("\nInvalid Commands");
    uart_puts("\n");
}

char *currentColors = "\x1b[37m";
char *currentBackgroundColors = "";
void setBackgroundColor(char *backgroundColors)
{
    if (cus_strcmp(backgroundColors, "black") == 0)
    {
        currentBackgroundColors = "\x1b[40m"; // 41 is red
    }
    else if (cus_strcmp(backgroundColors, "red") == 0)
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
    if (cus_strcmp(colors, "black") == 0)
    {
        currentColors = "\x1b[30m"; //
    }
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

void printBoardRevision(const unsigned int *mac)
{
    if (mac = 0x00A02082)
    {
        uart_puts("rpi-3B BCM2837 1GiB Sony UK");
    }
    else if (mac = 0x00900092)
    {
        uart_puts("rpi-Zero BCM2835 512MB Sony UK");
    }
    else if (mac = 0x00000010)
    {
        uart_puts("rpi-1B+ BCM2835");
    }
    else if (mac = 0x00a01041)
    {
        uart_puts("rpi-2B BCM2836 1GiB Sony UK");
    }
    else
    {
        uart_puts("rpi-4B BCM2711 2GiB Sony UK");
    }
}
char *tabHandler(char *cli_buffer)
{

    int i, x;
    int numCompletions = 0;
    int lastMatchingIndex = -1;
    int partialLength = cus_strlen(cli_buffer);

    for (i = 0; i < COMMANDS_SIZE; ++i)
    {
        int commandLength = 0;
        while (commands[i][commandLength] != '\0' && cli_buffer[commandLength] != '\0' && commands[i][commandLength] == cli_buffer[commandLength])
        {
            ++commandLength;
        }
        if (cli_buffer[commandLength] == '\0')
        {
            numCompletions++;
            lastMatchingIndex = i;
        }
    }

    if (numCompletions == 1)
    {
        return commands[lastMatchingIndex];
    }
    else if (numCompletions > 1) // more than 1 matches
    {
        for (i = 0; i < COMMANDS_SIZE; i++)
        {
            for (x = 0; x < cus_strlen(commands[i]) && x < partialLength; x++)
            {
                if (commands[i][x] != cli_buffer[x])
                {
                    break;
                }
            }
            if (x == partialLength)
            {
                uart_puts(commands[i]);
                uart_puts(" ");
            }
        }
        uart_puts("\n");
    }
    // Return an empty string if no completion or multiple completions
    cli_buffer[0] = '\0';
    return "";
}

void cli()
{

    static char cli_buffer[MAX_CMD_SIZE];
    static int index = 0;
    static char *token;

    // read and send back each char
    char c = uart_getc();
    // uart_dec(c);
    if (c != '\b' && c != 95 && c != 43)
    {
        uart_sendc(c);
    }
    // put into a buffer until got new line or get backspace character
    if (c != '\n' && c != '\b' && c != '\t' && c != 95 && c != 43)
    {

        cli_buffer[index] = c; // Store into the buffer
        index++;
    }

    // if user typein backspace it will delete the output and shift the index --
    if (c == '\b')
    {

        if (index <= 0)
        {
            // uart_puts(" ");
            return;
        }
        uart_puts("\b \b");
        index--;
        cli_buffer[index] = '\0';
    }

    if (c == '\t') // Working perfectly
    {

        if (index < 7) // try to delete all of the space when tabbing
        {
            for (int i = 0; i < 7 - index; i++)
            {
                uart_puts("\b \b"); // move back input space and move back
            }
        }
        if (index >= 7) // if more than 7 index it will use the function below to delete
        {
            for (int i = 0; i <= 7 - (index - 7); i++)
            {
                uart_puts("\b \b");
            }
        }
        char *completedCommand = tabHandler(cli_buffer);
        if (*completedCommand != '\0')
        {
            for (int i = 0; i < index; i++) // loops to delete out all output onscreen in order to replace it with completed command
            {
                uart_puts("\b \b");
            }
            uart_puts(completedCommand);
            cli_buffer[0] = '\0';
            index = cus_strlen(completedCommand);
            cus_strcpy(cli_buffer, completedCommand);
        }
    }

    if (c == 95)
    {
        if (currentHistoryList <= 0)
        {
            return;
        }
        currentHistoryList--;
        for (int i = 0; i < index; i++)
        {
            uart_puts("\b \b");
        }
        cus_strcpy(cli_buffer, history[currentHistoryList]);
        index = cus_strlen(cli_buffer);
        uart_puts(history[currentHistoryList]);
    }
    if (c == 43)
    {

        if (currentHistoryList > historyList)
        {
            return;
        }
        currentHistoryList++;
        for (int i = 0; i < index; i++)
        {
            uart_puts("\b \b");
        }
        cus_strcpy(cli_buffer, history[currentHistoryList]);
        index = cus_strlen(cli_buffer);
        uart_puts(history[currentHistoryList]);
    }
    else if (c == '\n')
    {
        uart_puts("\nGot commands: ");
        uart_puts(cli_buffer);
        cus_strcpy(history[historyList], cli_buffer);

        historyList++;
        currentHistoryList = historyList;
        uart_puts("\n");
        uart_puts("\n--------------------------------------------------");
        cli_buffer[index] = '\0';
        // char *token = cus_strtok(cli_buffer, " ");
        token = cus_strtok(cli_buffer, " ");

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
                else
                {
                    errors();
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
        // SETCOLOR FUNCTION
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
            }
            else if ((cus_strcmp(secondToken, "-t") != 0) && (cus_strcmp(secondToken, "-b") != 0))
            {
                errors();
            }
        }
        // CLEAR FUNCTION
        else if (cus_strcmp(token, "clear") == 0)
        {
            uart_puts("\e[1;1H\e[2J");
        }
        // SHOWINFO FUNCTION
        else if (cus_strcmp(token, "showinfo") == 0)
        {
            // Note: Board model and Board serial may give 0 values on QEMU.
            mBuf[0] = 30 * 4;       // Message Buffer Size in bytes
            mBuf[1] = MBOX_REQUEST; // Message Request Code

            mBuf[2] = 0x00030002; // TAG Identifier: Get clock rate (ARM clock)
            mBuf[3] = 8;          // Value buffer size in bytes
            mBuf[4] = 0;          // REQUEST CODE = 0
            mBuf[5] = 3;          // clock id: ARM system clock
            mBuf[6] = 0;          // clear output buffer (response data are mBuf[5] & mBuf[6])

            mBuf[7] = 0x00000001; // TAG Identifier: Get firmware revision
            mBuf[8] = 4;          // Value buffer size in bytes
            mBuf[9] = 0;          // REQUEST CODE = 0
            mBuf[10] = 0;         // clear output buffer (response data are mBuf[10])

            mBuf[11] = 0x00030002; // TAG Identifier: Get clock rate (UART clock)
            mBuf[12] = 8;          // Value buffer size in bytes
            mBuf[13] = 0;          // REQUEST CODE = 0
            mBuf[14] = 2;          // clock id: UART clock
            mBuf[15] = 0;          // clear output buffer (response data are mBuf[14] & mBuf[15])

            mBuf[16] = 0x00010002; // TAG Identifier: Get board revision
            mBuf[17] = 4;          // Value buffer size in bytes
            mBuf[18] = 0;          // REQUEST CODE = 0
            mBuf[19] = 0;          // clear output buffer (response data are mBuf[19])

            mBuf[20] = 0x00010003; // TAG Identifier: Get board MAC address
            mBuf[21] = 6 * 2;      // Value buffer size in bytes
            mBuf[22] = 0;          // REQUEST CODE = 0
            mBuf[23] = 0;          // clear output buffer (response data are mBuf[23])

            mBuf[24] = MBOX_TAG_LAST;

            if (mbox_call(ADDR(mBuf), MBOX_CH_PROP))
            {
                uart_puts("\nDATA: board revision = ");
                printBoardRevision(&mBuf[19]);

                uart_puts("\n+ Response Code in Message TAG (Board MAC address): ");
                uart_hex(mBuf[22]);
                uart_puts("\nDATA: board MAC address = ");
                printBoardRevision((unsigned char *)&mBuf[23]);
            }
            else
            {
                uart_puts("Unable to query!\n");
            }
        }
        else if (cus_strcmp(token, "clear") != 0 && cus_strcmp(token, "setcolor") != 0 && cus_strcmp(token, "help") != 0 && cus_strcmp(token, "showinfo") != 0)
        {
            errors();
        }
        // else
        // {
        //     errors();
        // }
        uart_puts(currentColors);
        uart_puts(currentBackgroundColors);
        uart_puts("\n--------------------------------------------------");
        uart_puts("\nNingOS:> ");
        for (int i = index; i >= 0; i--)
        {
            cli_buffer[i] = '\0';
        }

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
    uart_puts("\n--------------------------------------------------");
    uart_puts("\nNingOS:> ");

    // echo everything back
    while (1)
    {
        // read each char
        cli();
    }
}