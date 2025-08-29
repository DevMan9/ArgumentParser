#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "argumentparser.x"

const int NONE = 0;
const int ACCEPTS_ARGUMENTS = 0b1;

struct _ParseDetail
{
    struct Option *option;
    int passed;
    size_t argument_count;
    size_t arguments_size;
    char **arguments;
};

struct ParseDetails
{
    size_t detail_count;
    struct _ParseDetail *details;
    size_t positional_count;
    size_t positionals_size;
    char **positionals;
    size_t unrecognized_option_count;
    size_t unrecognized_options_size;
    char **unrecognized_options;
};

static inline void _CountOptions(struct Option *options, size_t *ret)
{
    (*ret) = 0;
    for (struct Option *option = options; option->long_option != NULL || option->short_option != NULL; option++)
    {
        (*ret)++;
    }
}

void CreateParseDetails(struct Option *options, struct ParseDetails **ret)
{
    (*ret) = malloc(sizeof(**ret));
    _CountOptions(options, &((*ret)->detail_count));
    (*ret)->details = malloc((*ret)->detail_count * sizeof(*((*ret)->details)));
    for (size_t i = 0; i < (*ret)->detail_count; i++)
    {
        struct _ParseDetail *detail = (*ret)->details + i;
        detail->option = options + i;
        detail->passed = 0;
        detail->argument_count = 0;
        detail->arguments_size = 16;
        detail->arguments = malloc(detail->arguments_size * sizeof(*(detail->arguments)));
        detail->arguments[0] = NULL;
    }
    (*ret)->positional_count = 0;
    (*ret)->positionals_size = 16;
    (*ret)->positionals = malloc((*ret)->positionals_size * sizeof(*((*ret)->positionals)));
    (*ret)->positionals[0] = NULL;

    (*ret)->unrecognized_option_count = 0;
    (*ret)->unrecognized_options_size = 16;
    (*ret)->unrecognized_options = malloc((*ret)->unrecognized_options_size * sizeof(*((*ret)->unrecognized_options)));
    (*ret)->unrecognized_options[0] = NULL;
}

void DestroyParseDetails(struct ParseDetails **parse_details_address)
{
    struct ParseDetails *parse_details = (*parse_details_address);
    for (size_t i = 0; i < parse_details->detail_count; i++)
    {
        struct _ParseDetail *detail = parse_details->details + i;
        for (char **argument = detail->arguments; *argument != NULL; argument++)
        {
            free(*argument);
        }
        free(detail->arguments);
    }
    free(parse_details->details);

    for (char **argument = parse_details->positionals; *argument != NULL; argument++)
    {
        free(*argument);
    }
    free(parse_details->positionals);

    for (char **argument = parse_details->unrecognized_options; *argument != NULL; argument++)
    {
        free(*argument);
    }
    free(parse_details->unrecognized_options);

    free(parse_details);
    (*parse_details_address) = NULL;
}

void _ParseDetailAddArgument(struct _ParseDetail *parse_detail, char *argument)
{
    parse_detail->arguments[parse_detail->argument_count] = strdup(argument);
    parse_detail->argument_count++;
    if (parse_detail->argument_count >= parse_detail->arguments_size)
    {
        parse_detail->arguments_size <<= 1;
        parse_detail->arguments = realloc(parse_detail->arguments, parse_detail->arguments_size * sizeof(*(parse_detail->arguments)));
    }
    parse_detail->arguments[parse_detail->argument_count] = NULL;
}

void _ParseDetailsAddPositional(struct ParseDetails *parse_details, char *positional)
{
    parse_details->positionals[parse_details->positional_count] = strdup(positional);
    parse_details->positional_count++;
    if (parse_details->positional_count >= parse_details->positionals_size)
    {
        parse_details->positionals_size <<= 1;
        parse_details->positionals = realloc(parse_details->positionals, parse_details->positionals_size * sizeof(*(parse_details->positionals)));
    }
    parse_details->positionals[parse_details->positional_count] = NULL;
}

void _ParseDetailsAddUnrecognized(struct ParseDetails *parse_details, char *unrecognized_option)
{
    parse_details->unrecognized_options[parse_details->unrecognized_option_count] = strdup(unrecognized_option);
    parse_details->unrecognized_option_count++;
    if (parse_details->unrecognized_option_count >= parse_details->unrecognized_options_size)
    {
        parse_details->unrecognized_options_size <<= 1;
        parse_details->unrecognized_options = realloc(parse_details->unrecognized_options, parse_details->unrecognized_options_size * sizeof(*(parse_details->unrecognized_options)));
    }
    parse_details->unrecognized_options[parse_details->unrecognized_option_count] = NULL;
}

void _ParseDetailsFindDetail(struct ParseDetails *parse_details, char *option, struct _ParseDetail **ret)
{
    (*ret) = NULL;
    char *option_sans = option;
    int use_long_option = 0;
    if (option[1] == '-')
    {
        use_long_option = 1;
        option_sans += 2;
    }
    else
    {
        use_long_option = 0;
        option_sans += 1;
    }
    for (int i = 0; i < parse_details->detail_count; i++)
    {
        char *detail_option = NULL;
        if (use_long_option)
        {
            detail_option = parse_details->details[i].option->long_option;
        }
        else
        {
            detail_option = parse_details->details[i].option->short_option;
        }
        if (detail_option == NULL)
        {
            goto for_loop_end;
        }
        if (strcmp(option_sans, detail_option) == 0)
        {
            (*ret) = parse_details->details + i;
            break;
        }

    for_loop_end:
        continue;
    }
}

void ParseArguments(int argc, char **argv, struct Option *options, struct ParseDetails **ret)
{
    CreateParseDetails(options, ret);

    struct _ParseDetail *previous_option_detail = NULL;
    int more_options = 1;
    for (char **arg = argv + 1; *arg != NULL; arg++)
    {
        if (previous_option_detail != NULL && (previous_option_detail->option->flags & ACCEPTS_ARGUMENTS))
        {
            goto option_argument;
        }
        goto regular_argument;
    regular_argument:
        if (more_options && (*arg)[0] == '-')
        {
            // Option
            // Check for "--"
            if (strcmp(*arg, "--") == 0)
            {
                more_options = 0;
                goto for_loop_end;
            }

            _ParseDetailsFindDetail((*ret), (*arg), &previous_option_detail);
            if (previous_option_detail == NULL)
            {
                _ParseDetailsAddUnrecognized((*ret), (*arg));
                goto for_loop_end;
            }
            previous_option_detail->passed = 1;
        }
        else
        {
            // Positional argument
            _ParseDetailsAddPositional((*ret), *arg);
        }
        goto for_loop_end;

    option_argument:
        _ParseDetailAddArgument(previous_option_detail, *arg);
        previous_option_detail = NULL;
        goto for_loop_end;

    for_loop_end:
        continue;
    }
}

void ParseDetailsWasOptionPassed(struct ParseDetails *parse_details, char *option, int *ret)
{
    struct _ParseDetail *detail = NULL;
    _ParseDetailsFindDetail(parse_details, option, &detail);
    if (detail == NULL)
    {
        (*ret) = -1;
        goto end;
    }
    (*ret) = detail->passed;

end:
    return;
}

void ParseDetailsGetOptionArguments(struct ParseDetails *parse_details, char *option, char ***ret)
{
    struct _ParseDetail *detail = NULL;
    _ParseDetailsFindDetail(parse_details, option, &detail);
    if (detail == NULL)
    {
        (*ret) = NULL;
        goto end;
    }
    (*ret) = detail->arguments;

end:
    return;
}

void ParseDetailsGetPositionalArguments(struct ParseDetails *parse_details, char ***ret)
{
    (*ret) = parse_details->positionals;
}

void ParseDetailsGetUnrecognizedOptions(struct ParseDetails *parse_details, char ***ret)
{
    (*ret) = parse_details->unrecognized_options;
}
