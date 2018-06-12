#ifndef DROPBOX_SERVER_LOGIN_PARSER_HPP
#define DROPBOX_SERVER_LOGIN_PARSER_HPP

#include <boost/program_options.hpp>

namespace program_options = boost::program_options;

class ServerLoginParser {
public:
    ServerLoginParser();


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
     * Returns true if the server is the primary node, false otherwise. Throws if ParseInput wasn't called before
     */
    bool isPrimary();

    /**
     * Returns the specified port, throws if ParseInput wasn't called before
     */
    int64_t GetPort();

private:
    program_options::options_description description_;
    program_options::positional_options_description positional_description_;
    program_options::variables_map variables_map_;
};


#endif //DROPBOX_SERVER_LOGIN_PARSER_HPP
