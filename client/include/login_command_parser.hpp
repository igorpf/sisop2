#ifndef SISOP2_CLIENT_INCLUDE_LOGIN_COMMAND_PARSER_HPP
#define SISOP2_CLIENT_INCLUDE_LOGIN_COMMAND_PARSER_HPP

#include <string>

#include <boost/program_options.hpp>

namespace program_options = boost::program_options;

// TODO(jfguimaraes) Traduzir aqui
/**
 * Class for parsing and validating user input, making parameters available through get functions
 * and making it possible to show a helper message if the user specifies so
 *
 * Usage:
 * Create an instance of the class and pass the argc and argv variables to ParseInput which may throw
 * if there are missing/wrong arguments, then call ValidateInput which may throw if the input is invalid
 * (more on the documentation of each function). The ShowHelpMessage function may be called before or
 * after ValidateInput and will show a help message if the user specified so and return true (so you
 * can finish execution, otherwise returns false and execution may continue. After all these steps
 * the arguments will be available through get functions.
 */
class LoginCommandParser {
public:
    /**
     * Initializes the arguments description
     */
    LoginCommandParser();

    /**
     * Verifies that the input is complete and on the right format
     * @param argc Program's argument count
     * @param argv Program's argument vector
     */
    void ParseInput(int argc, char **argv);

    /**
     * Verifies that each variable is on the desired format:
     *
     * userid: longer than 4 characters
     * hostname: valid IPv4 or IPv6 address
     * port: integer between 0 and 65536
     *
     * Throws if ParseInput wasn't called before
     */
    void ValidateInput();

    /**
     * Shows the help message if --help specified and returns true so program might be finished,
     * returns false otherwise
     * @return True if help message is shown, false otherwise
     *
     * Throws if ParseInput wasn't called before
     */
    bool ShowHelpMessage();

    /**
     * Returns the specified userid, throws if ParseInput wasn't called before
     */
    std::string GetUserid();

    /**
     * Returns the specified hostname, throws if ParseInput wasn't called before
     */
    std::string GetHostname();

    /**
     * Returns the specified port, throws if ParseInput wasn't called before
     */
    int64_t GetPort();

private:
    program_options::options_description description_;
    program_options::positional_options_description positional_description_;
    program_options::variables_map variables_map_;
};

#endif // SISOP2_CLIENT_INCLUDE_LOGIN_COMMAND_PARSER_HPP
