#ifndef UMPS_SERVICES_INCREMENTER_OPTIONS_HPP
#define UMPS_SERVICES_INCREMENTER_OPTIONS_HPP
#include <memory>
#include "umps/logging/level.hpp"
namespace UMPS::Authentication
{
 class ZAPOptions;
}
namespace UMPS::Services::Incrementer
{
/// @class Options "options.hpp" "umps/services/incrementer/options.hpp"
/// @brief The options for controlling the incrementer.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class Options
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    Options();
    /// @brief Copy constructor.
    /// @param[in] options  The options from which to initialize
    ///                        this class. 
    Options(const Options &options);
    /// @brief Move constructor.
    /// @param[in] options  The options from which to initialize this
    ///                        class.  On exit, option's behavior is
    ///                        undefined. 
    Options(Options &&options) noexcept;
    /// @}

    /// @name Operators
    /// @{

    /// @brief Copy assignment.
    /// @param[in] options   The options class to copy to this.
    /// @result A deep copy of options.
    Options& operator=(const Options &options);
    /// @brief Move assignment.
    /// @param[in,out] options  The options whose memory will be moved to
    ///                            this.  On exit, options's behavior is
    ///                            undefined.
    /// @result The memory from options moved to this.
    Options& operator=(Options &&options) noexcept;
    /// @}

    /// @brief Loads the options from an initialization file.
    /// @param[in] fileName   The name of the initialization file.
    /// @param[in] section    The section of the initialization file with the
    ///                       information to be parsed.  This will likely
    ///                       be of the form "Counter.Name" where Name is the
    ///                       name of the item to count e.g., "Pick",
    ///                       "Amplitude", etc.
    /// @throws std::invalid_argument if the initialization file does not,
    ///         exist cannot be parsed, does not have the specified section,
    ///         or has incorrect information.
    void parseInitializationFile(const std::string &fileName,
                                 const std::string &section);

    /// @name Required Options
    /// @{

    /// @brief Sets the proxy's address from which the server will connect.
    /// @param[in] address  The address from which the server connect to this
    ///                     incrementer service.
    /// @note This address does not need to be known by the clients.
    //void setServerAccessAddress(const std::string &address);
    /// @result The address from which the server will access the service.
    /// @throws std::runtime_error if \c haveServerAccessAddress() is false.
    //[[nodiscard]] std::string getServerAccessAddress() const;
    /// @result True indicates that the server access address was set.
    //[[nodiscard]] bool haveServerAccessAddress() const noexcept;

    /// @brief Sets the proxy's address to which the clients will connect.
    /// @param[in] address  The address from which clients will connect to this
    ///                     incrementer service.
    void setClientAccessAddress(const std::string &address);
    /// @result The address from which clients will access the service.
    /// @throws std::runtime_error if \c haveClientAccessAddress() is false.
    [[nodiscard]] std::string getClientAccessAddress() const; 
    /// @result True indicates that the client access address was set.
    [[nodiscard]] bool haveClientAccessAddress() const noexcept;
    /// @}

    /// @name Optional Options
    /// @{

    /// @brief Sets the sqlite3 file name.
    /// @param[in] fileName  The name of the sqlite3 file.
    void setSqlite3FileName(const std::string &fileName);
    /// @result The sqlite3 file containing the items to be incremented.
    [[nodiscard]] std::string getSqlite3FileName() const noexcept;

    /// @brief Sets the increment for all items.
    /// @param[in] increment  The amount by which to increment the counter.
    /// @throws std::invalid_argument if increment is not positive.
    void setIncrement(int increment);
    /// @result The increment.
    [[nodiscard]] int getIncrement() const noexcept;

    /// @param[in] initialValue   The initial value for all items to be
    ///                           incremented.
    /// @note The next value returned will be this plus the increment.
    void setInitialValue(int32_t initialValue) noexcept;
    /// @result The initial value of the incrementer.
    [[nodiscard]] int64_t getInitialValue() const noexcept;

    /// @brief Sets the verbosity. 
    /// @param[in] verbosity   The verbosity.
    void setVerbosity(UMPS::Logging::Level verbosity) noexcept;
    /// @result The verbosity of the conter.
    [[nodiscard]] UMPS::Logging::Level getVerbosity() const noexcept;

    /// @brief Sets the ZeroMQ Authentication Protocol options.
    /// @param[in] zapOptions  The ZAP options.
    void setZAPOptions(
        const UMPS::Authentication::ZAPOptions &zapOptions) noexcept; 
    /// @result The ZAP options.
    [[nodiscard]] 
    UMPS::Authentication::ZAPOptions getZAPOptions() const noexcept;
    /// @}

    /// @name Destructors
    /// @{

    /// @brief Resets the class.
    void clear() noexcept;
    /// @brief Destructor.
    ~Options();
    /// @}
private:
    class OptionsImpl;
    std::unique_ptr<OptionsImpl> pImpl;    
};
}
#endif
