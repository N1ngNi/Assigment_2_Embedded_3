// -----------------------------------main.c -------------------------------------
#include "uart.h"
#include "mbox.h"

#define MAX_CMD_SIZE 100
#define COMMANDS_SIZE 4
char *commands[] = {"help", "clear", "setcolor", "showinfo"};
char *history[100][100];
static int historyList = 0;
static int currentHistoryList = 0;
static int identicalMatchNum = 0;
static int identicalLock = 0;
int currentBaud = 115200;
int currentData = 8;
int currentStop = 1;
int currentParity = 1;
int currentHS = 0;
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

int cus_parseint(char *str)
{
    int result = 0; // Initialize result
    int sign = 1;   // Initialize sign as positive
    int i = 0;      // Initialize index of first digit

    // If number is negative, then update sign
    if (str[0] == '-')
    {
        sign = -1;
        i++; // Also update index of first digit
    }

    // Iterate through all digits and update the result
    for (; str[i] != '\0'; ++i)
    {
        if (str[i] < '0' || str[i] > '9')
        {
            return -1; // return -1 when non-digit character is encountered
        }
        result = result * 10 + str[i] - '0';
    }

    // Return result with sign
    return sign * result;
}
//------------------main function-----------------------------------------------------
void errors()
{
    uart_puts("\nInvalid Commands");
    uart_puts("\n");
}
int delete(int index)
{

    if (index > 0)
    {
        uart_puts("\b \b");
        index--;
    }
    return (index);
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
void printMacAddress(const unsigned char *mac)
{
    for (int i = 5; i >= 0; i--)
    {
        uart_dec(mac[i] % 16); // Print last digit
        uart_dec(mac[i] / 16); // Print second last digit
        if (i > 0)
        {
            uart_puts(":");
        }
    }
}
void uartShowSetting()
{
    // BAUD RATE
    uart_puts("Baudrates: ");
    uart_dec(currentBaud);
    uart_puts("\n");

    // DATA BIT
    uart_puts("Databits: ");
    uart_dec(currentData);
    uart_puts("\n");
    // STOP BIT
    uart_puts("Stopbits: ");
    uart_dec(currentStop);
    uart_puts("\n");

    // PARITY BIT

    if (currentParity == 1)
    {
        uart_puts("Parity bit : none \n");
    }
    else if (currentParity == 2)
    {
        uart_puts("Parity bit : odd \n");
    }
    else if (currentParity == 3)
    {
        uart_puts("Parity bit : even \n");
    }
    // HANDSHAKING
    if (currentHS == 0)
    {
        uart_puts("handshaking : None \n");
    }
    else if (currentHS == 1)
    {
        uart_puts("Handshaking : RTS/CTS \n");
    }
}
char *tabHandler(char *cli_buffer, int currentMatch)
{
    int i, x;
    int numCompletions = 0;
    int lastMatchingIndex = -1;
    int partialLength = cus_strlen(cli_buffer);
    char *identicalCommands[] = {"setcolor", "showinfo"};
    for (i = 0; i < COMMANDS_SIZE; ++i)
    {
        int commandLength = 0;

        while (commands[i][commandLength] != '\0' && cli_buffer[commandLength] != '\0' && commands[i][commandLength] == cli_buffer[commandLength])
        {
            ++commandLength;
            if (cli_buffer[commandLength] == '\0')
            {
                numCompletions++;
                lastMatchingIndex = i;
            }
        }
    }

    if (numCompletions == 1 && identicalLock == 0)
    {
        return commands[lastMatchingIndex];
    }
    else if (numCompletions > 1) // more than 1 matches
    {
        identicalLock = 1;
        return identicalCommands[currentMatch];
    }
    // Return an empty string if no completion or multiple completions
    cli_buffer[0] = '\0';
    return "";
}

cli()
{

    static char cli_buffer[MAX_CMD_SIZE];
    static int index = 0;
    static char *token;

    // read and send back each char
    char c = uart_getc();
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
        identicalLock = 0;
        if (index > 0)
        {
            uart_puts("\b \b");
            index--;
            cli_buffer[index] = '\0';
        }
    }

    if (c == '\t') // Working perfectly
    {

        identicalMatchNum = identicalMatchNum ^ 1;

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
        if (identicalLock == 1)
        {
            for (int i = 1; i < index; i++)
            {
                cli_buffer[i] = '\0';
            }
        }
        char *completedCommand = tabHandler(cli_buffer, identicalMatchNum);
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
        uart_puts(cli_buffer);
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
        uart_puts(cli_buffer);
    }
    else if (c == '\n')
    {
        identicalLock = 0;
        uart_puts("\nGot commands: ");
        uart_puts(cli_buffer);

        cus_strcpy(history[historyList], cli_buffer);
        historyList++;
        currentHistoryList = historyList;

        uart_puts("\n");
        uart_puts("\n--------------------------------------------------");
        cli_buffer[index] = '\0';
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
                    uart_puts("\nclear                     - Clear screen.");
                }
                else if (cus_strcmp(secondToken, "setcolor") == 0)
                {
                    uart_puts("\nsetcolor                  - Set text color, and/or background color of the console to one of the following colors: BLACK, RED, GREEN, YELLOW, BLUE, PURPLE, CYAN, WHITE");
                    uart_puts("\n-t <text color> \n-b <background color>");
                }
                else if (cus_strcmp(secondToken, "showinfo") == 0)
                {
                    uart_puts("\nshowinfo                  - Show board revision and board MAC address");
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
            mBuf[0] = 12 * 4;
            mBuf[1] = MBOX_REQUEST;
            // get board revision
            mBuf[2] = 0x00010002; // TAG Identifier: Get board revision
            mBuf[3] = 4;          // Value buffer size in bytes
            mBuf[4] = 0;          // REQUEST CODE = 0
            mBuf[5] = 0;          // Clear output buffer

            mBuf[6] = 0x00010003; // TAG Identifier: MAC address
            mBuf[7] = 6;          // Value buffer size in bytes
            mBuf[8] = 0;          // REQUEST CODE = 0
            mBuf[9] = 0;          // Clear output buffer
            mBuf[11] = MBOX_TAG_LAST;

            if (mbox_call(ADDR(mBuf), MBOX_CH_PROP))
            {
                uart_puts("\nBoard Revision: ");
                printBoardRevision(mBuf[5]);
                uart_puts("\nBoard MAC Address: ");
                printMacAddress((unsigned char *)&mBuf[9]);
                uart_puts("\n");
                uartShowSetting();
            }
            else
            {
                uart_puts("Unable to query!\n");
            }
        }

        else if (cus_strcmp(token, "baudrate") == 0)
        {

            uart_puts("\nEnter baudrate: ");
            static char baudrateBuffer[10];
            static int baudrateIndex = 0;
            static char baudrateChar;

            do
            {
                baudrateChar = uart_getc();
                // echo the character

                if (baudrateChar != '\n' && baudrateChar != '\r' && baudrateChar != '\b')
                {
                    uart_sendc(baudrateChar);
                    baudrateBuffer[baudrateIndex] = baudrateChar;
                    baudrateIndex++;
                }
                if (baudrateChar == '\b')
                {
                    baudrateIndex = delete (baudrateIndex);
                }
            } while (baudrateChar != '\n' && baudrateChar != '\r' && baudrateIndex < sizeof(baudrateBuffer) - 1);
            baudrateBuffer[baudrateIndex] = '\0';        // null-terminate the string
            int baudrate = cus_parseint(baudrateBuffer); // convert string to integer

            uart_init(baudrate, currentData, currentStop, currentParity, currentHS);
            currentBaud = baudrate;
            for (int i = 0; i < 10; i++)
            { // reset buffer
                baudrateBuffer[i] = 0;
            }
            baudrateIndex = 0;
        }

        else if (cus_strcmp(token, "databit") == 0)
        {
            uart_puts("\nEnter data bit (5 | 6 | 7 | 8): ");
            static char databitBuffer[2];
            static int databitIndex = 0;
            static char databitChar;
            do
            {
                databitChar = uart_getc();
                if (databitChar != '\n' && databitChar != '\r')
                {
                    uart_sendc(databitChar);
                    databitBuffer[databitIndex] = databitChar;
                    databitIndex++;
                }
            } while (databitChar != '\n' && databitChar != '\r' && databitIndex < sizeof(databitBuffer) - 1);
            databitBuffer[databitIndex] = '\0';
            int databit = cus_parseint(databitBuffer);
            if (databit != 5 && databit != 6 && databit != 7 && databit != 8)
            {
                uart_puts("\nWarning! Invalid Input.\n");
            }
            else
            {
                uart_init(currentBaud, databit, currentStop, currentParity, currentHS);
                currentData = databit;
            }
            for (int i = 0; i < 2; i++)
            {
                databitBuffer[i] = 0;
            }
            databitIndex = 0;
        }
        else if (cus_strcmp(token, "stopbit") == 0)
        {
            uart_puts("\nEnter stop bit (1 | 2): ");
            static char stopbitBuffer[2];
            static int stopbitIndex = 0;
            static char stopbitChar;
            do
            {
                stopbitChar = uart_getc();
                if (stopbitChar != '\n' && stopbitChar != '\r')
                {
                    uart_sendc(stopbitChar);
                    stopbitBuffer[stopbitIndex] = stopbitChar;
                    stopbitIndex++;
                }
            } while (stopbitChar != '\n' && stopbitChar != '\r' && stopbitIndex < sizeof(stopbitBuffer) - 1);
            stopbitBuffer[stopbitIndex] = '\0';
            int stopbit = cus_parseint(stopbitBuffer);

            if (stopbit != 1 && stopbit != 2)
            {
                uart_puts("\nWarning! Invalid Input.\n");
            }
            else
            {
                uart_init(currentBaud, currentData, stopbit, currentParity, currentHS);
                currentStop = stopbit;
            }
            for (int i = 0; i < 2; i++)
            {
                stopbitBuffer[i] = 0;
            }
            stopbitIndex = 0;
        }
        else if (cus_strcmp(token, "parity") == 0)
        {
            uart_puts("\nEnter parity choice (none | even | odd): ");
            static char parityBuffer[5]; // Increase buffer size to 2
            static int parityIndex = 0;
            static char parityChar;
            int parityInput = 0;
            do
            {
                parityChar = uart_getc();
                if (parityChar != '\n' && parityChar != '\r' && parityChar != '\b')
                {
                    uart_sendc(parityChar); // echo the character
                    parityBuffer[parityIndex] = parityChar;
                    parityIndex++;
                }
                if (parityChar == '\b')
                {
                    parityIndex = delete (parityIndex);
                }
            } while (parityChar != '\n' && parityChar != '\r' && parityIndex < sizeof(parityBuffer) - 1);
            parityBuffer[parityIndex] = '\0';
            if (cus_strcmp(parityBuffer, "none") != 0 && cus_strcmp(parityBuffer, "odd") != 0 && cus_strcmp(parityBuffer, "even") != 0)
            {
                uart_puts("\nWarning! Invalid Input.\n");
            }

            if (cus_strcmp(parityBuffer, "none") == 0)
            {
                parityInput = 1;
            }
            else if (cus_strcmp(parityBuffer, "odd") == 0)
            {
                parityInput = 2;
            }
            else
            {
                parityInput = 3; // even
            }

            uart_init(currentBaud, currentData, currentStop, parityInput, currentHS);
            currentParity = parityInput;
            for (int i = 0; i < 2; i++)
            {
                parityBuffer[i] = 0;
            }
            parityIndex = 0;
        }
        else if (cus_strcmp(token, "handshaking") == 0)
        {
            uart_puts("\nEnter handshaking choice (on | off): ");
            static char hsBuffer[3];
            static int hsIndex = 0;
            static char hsChar;
            do
            {
                hsChar = uart_getc();
                if (hsChar != '\n' && hsChar != '\r' && hsChar != '\b')
                {
                    uart_sendc(hsChar); // echo the character
                    hsBuffer[hsIndex] = hsChar;
                    hsIndex++;
                }
                if (hsChar == '\b')
                {
                    hsIndex = delete (hsIndex);
                    hsBuffer[hsIndex] = '\0';
                }
            } while (hsChar != '\n' && hsChar != '\r' && hsIndex < sizeof(hsBuffer) - 1);
            hsBuffer[hsIndex] = '\0'; // null-terminate the string
            if (cus_strcmp(hsBuffer, "off") != 0 && cus_strcmp(hsBuffer, "on") != 0)
            {
                uart_puts("\nWarning! Invalid Input.\n");
            }

            int hsInput = 0;
            if (cus_strcmp(hsBuffer, "on") == 0)
            {
                hsInput = 1;
            }
            else
            {
                hsInput = 0; // none
            }
            uart_init(currentBaud, currentData, currentStop, currentParity, hsInput);
            currentHS = hsInput;
            for (int i = 0; i < 7; i++)
            {
                hsBuffer[i] = 0;
            }
            hsIndex = 0;
        }
        else if (cus_strcmp(token, "clear") != 0 && cus_strcmp(token, "setcolor") != 0 && cus_strcmp(token, "help") != 0 && cus_strcmp(token, "showinfo") != 0 && cus_strcmp(token, "baudrate") != 0 && cus_strcmp(token, "databit") != 0 && cus_strcmp(token, "stopbit") != 0 && cus_strcmp(token, "parity") != 0 && cus_strcmp(token, "handshaking") != 0)
        {
            errors();
        }

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
    return cli_buffer[index];
}

//------------------------------------------------------main--------------------------------------
int main()
{
    // intitialize UART

    uart_init(currentBaud, currentData, currentStop, currentParity, currentHS);

    // say hello
    uart_puts("\e[1;1H\e[2J");
    uart_puts("\n ______ ______ ______ _______ ___  _  _    ___   ___ \n"
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