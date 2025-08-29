#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../source/argumentparser.x"

int main(int argc, char **argv)
{
    struct Option options[] = {
        {.long_option = "version", .short_option = NULL, .flags = NONE, .help_message = "Displays the version."},
        {.long_option = "help", .short_option = NULL, .flags = NONE, .help_message = "Displays help."},
        {.long_option = "output", .short_option = "o", .flags = ACCEPTS_ARGUMENTS, .help_message = "Write output to file."},
        {.long_option = NULL, .short_option = NULL} // Terminator
    };

    // Test 1
    printf("Testing --help\n");
    char *argv2[] = {argv[0], "--help", NULL};
    int argc2 = 2;
    struct ParseDetails *parse_details = NULL;
    ParseArguments(argc2, argv2, options, &parse_details);

    int flag = 0;
    ParseDetailsWasOptionPassed(parse_details, "--help", &flag);
    assert(flag == 1);

    DestroyParseDetails(&parse_details);

    // Test 2
    printf("Testing no --help\n");
    char *argv3[] = {argv[0], NULL};
    int argc3 = 1;
    parse_details = NULL;
    ParseArguments(argc3, argv3, options, &parse_details);

    flag = 0;
    ParseDetailsWasOptionPassed(parse_details, "--help", &flag);
    assert(flag == 0);

    DestroyParseDetails(&parse_details);

    // Test 3
    printf("Testing --output\n");
    char *argv4[] = {argv[0], "--output", "output_file", NULL};
    int argc4 = 1;
    parse_details = NULL;
    ParseArguments(argc4, argv4, options, &parse_details);

    char **arguments = NULL;
    ParseDetailsGetOptionArguments(parse_details, "--output", &arguments);
    assert(strcmp(arguments[0], "output_file") == 0);

    DestroyParseDetails(&parse_details);

    // Test 4
    printf("Testing --output multiple\n");
    char *argv5[] = {argv[0], "--output", "first_file", "--output", "second_file", "--output", "third_file", NULL};
    int argc5 = 1;
    parse_details = NULL;
    ParseArguments(argc5, argv5, options, &parse_details);

    arguments = NULL;
    ParseDetailsGetOptionArguments(parse_details, "--output", &arguments);
    assert(strcmp(arguments[0], "first_file") == 0);
    assert(strcmp(arguments[1], "second_file") == 0);
    assert(strcmp(arguments[2], "third_file") == 0);

    DestroyParseDetails(&parse_details);

    // Test 5
    printf("Testing positionals\n");
    char *argv6[] = {argv[0], "--output", "first_file", "positional_1", "--output", "third_file", "positional_2", NULL};
    int argc6 = 7;
    parse_details = NULL;
    ParseArguments(argc6, argv6, options, &parse_details);

    arguments = NULL;
    ParseDetailsGetPositionalArguments(parse_details, &arguments);
    assert(strcmp(arguments[0], "positional_1") == 0);
    assert(strcmp(arguments[1], "positional_2") == 0);

    DestroyParseDetails(&parse_details);

    // Test 6
    printf("Testing --\n");
    char *argv7[] = {argv[0], "--output", "first_file", "positional_1", "--", "--output", "third_file", "positional_2", NULL};
    int argc7 = 8;
    parse_details = NULL;
    ParseArguments(argc7, argv7, options, &parse_details);

    arguments = NULL;
    ParseDetailsGetPositionalArguments(parse_details, &arguments);
    assert(strcmp(arguments[0], "positional_1") == 0);
    assert(strcmp(arguments[1], "--output") == 0);
    assert(strcmp(arguments[2], "third_file") == 0);
    assert(strcmp(arguments[3], "positional_2") == 0);

    DestroyParseDetails(&parse_details);

    // Test 7
    printf("Testing -- as option argument\n");
    char *argv8[] = {argv[0], "--output", "--", "--output", "second_file", "--output", "third_file", NULL};
    int argc8 = 1;
    parse_details = NULL;
    ParseArguments(argc8, argv8, options, &parse_details);

    arguments = NULL;
    ParseDetailsGetOptionArguments(parse_details, "-o", &arguments);
    assert(strcmp(arguments[0], "--") == 0);
    assert(strcmp(arguments[1], "second_file") == 0);
    assert(strcmp(arguments[2], "third_file") == 0);

    DestroyParseDetails(&parse_details);

    // Test 8
    printf("Testing unrecognized options\n");
    char *argv9[] = {argv[0], "--bad", "-b", "--output", "file_name", "--poo", "positional", NULL};
    int argc9 = 7;
    parse_details = NULL;
    ParseArguments(argc9, argv9, options, &parse_details);

    arguments = NULL;
    ParseDetailsGetUnrecognizedOptions(parse_details, &arguments);
    assert(strcmp(arguments[0], "--bad") == 0);
    assert(strcmp(arguments[1], "-b") == 0);
    assert(strcmp(arguments[2], "--poo") == 0);

    DestroyParseDetails(&parse_details);

    printf("---All tests passed!---\n");
    return 0;
}