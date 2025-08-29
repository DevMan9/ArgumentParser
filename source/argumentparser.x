#ifndef STANDARD_ARGUMENT_PARSER_X_0
#define STANDARD_ARGUMENT_PARSER_X_0

// Flag bit constants
extern const int NONE;
extern const int ACCEPTS_ARGUMENTS;

struct Option
{
    char *long_option;
    char *short_option;
    int flags;
    char *help_message;
};

struct ParseDetails;

/*
Takes argc and argv and parses them according to options.  Raturns ParseDetails to *ret.
options should be an array of struct Option ending with an option where both .long_option == NULL and .short_option == NULL.
example options:
struct Option options[] = {
        {.long_option = "version", .short_option = NULL, .flags = NONE, .help_message = "Displays the version."},
        {.long_option = "help", .short_option = NULL, .flags = NONE, .help_message = "Displays help."},
        {.long_option = "output", .short_option = "o", .flags = ACCEPTS_ARGUMENTS, .help_message = "Write output to file."},
        {.long_option = NULL, .short_option = NULL} // Terminator
    };

.long_options are automatically prefixed with "--" during parsing, .short_options with "-".
*/
void ParseArguments(int argc, char **argv, struct Option *options, struct ParseDetails **ret);

// Destroys the given ParseDetails and NULLs the address after.
void DestroyParseDetails(struct ParseDetails **parse_details_address);

// Gets whether or not the the option was passed.
void ParseDetailsWasOptionPassed(struct ParseDetails *parse_details, char *option, int *ret);

// Gets all arguments passed to this option.
void ParseDetailsGetOptionArguments(struct ParseDetails *parse_details, char *option, char ***ret);

// Gets all positional arguments.
void ParseDetailsGetPositionalArguments(struct ParseDetails *parse_details, char ***ret);

// Gets all unrecognized options.
void ParseDetailsGetUnrecognizedOptions(struct ParseDetails *parse_details, char ***ret);

#endif // STANDARD_ARGUMENT_PARSER_X_0