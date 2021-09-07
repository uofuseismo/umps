#ifndef URTS_MODULES_INCREMENTER_PARAMETERS_HPP
#define URTS_MODULES_INCREMENTER_PARAMETERS_HPP
#include <memory>
namespace URTS::Modules::Incrementer
{
/// @class Parameters "parameters.hpp" "urts/modules/incrementer/parameters.hpp"
/// @brief The parameters for controlling the item incrementer.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class Parameters
{
public:
    /// @name Constructors
    /// @{
    /// @brief Constructor.
    Parameters();
    /// @brief Copy constructor.
    /// @param[in] parameters  The parameters from which to initialize
    ///                        this class. 
    Parameters(const Parameters &parameters);
    /// @brief Move constructor.
    /// @param[in] parameters  The parameters from which to initialize this
    ///                        class.  On exit, parameter's behavior is
    ///                        undefined. 
    Parameters(Parameters &&parameters) noexcept;
    /// @}

    /// @name Operators
    /// @{
    /// @brief Copy assignment.
    /// @param[in] parameters   The parameters class to copy to this.
    /// @result A deep copy of parameters.
    Parameters& operator=(const Parameters &parameters);
    /// @brief Move assignment.
    /// @param[in,out] parameters  The parameters whose memory will be moved to
    ///                            this.  On exit, parameters's behavior is
    ///                            undefined.
    /// @result The memory from parameters moved to this.
    Parameters& operator=(Parameters &&parameters) noexcept;
    /// @}

    /// @brief Loads the parameters from an initialization file.
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

    /// @name Required Parameters
    /// @{
    /// @brief Sets the name of the item to be incremented.
    /// @param[in] name  The name of the item we are incrementing.
    /// @throws std::invalid_argument if name is empty.
    void setName(const std::string &name);
    /// @result The name of the item which we are incrementing.
    [[nodiscard]] std::string getName() const;
    /// @result True indicates that the name was set.
    [[nodiscard]] bool haveName() const noexcept;

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
    /// @throws std::invalid_argument if the address is blank.
    void setClientAccessAddress(const std::string &address);
    /// @result The address from which clients will access the service.
    /// @throws std::runtime_error if \c haveClientAccessAddress() is false.
    [[nodiscard]] std::string getClientAccessAddress() const; 
    /// @result True indicates that the client access address was set.
    [[nodiscard]] bool haveClientAccessAddress() const noexcept;
    /// @}

    /// @name Optional Parameters
    /// @{
    /// @brief Sets the increment.
    /// @param[in] increment  The amount by which to increment the counter.
    /// @throws std::invalid_argument if increment is not positive.
    void setIncrement(int increment);
    /// @result The increment.
    [[nodiscard]] uint64_t getIncrement() const noexcept;

    /// @param[in] initialValue   The initial value of the incrementer.
    /// @note The next value returned will be this plus the increment.
    void setInitialValue(uint32_t initialValue) noexcept;
    /// @result The initial value of the incrementer.
    [[nodiscard]] uint64_t getInitialValue() const noexcept;
    /// @}

    /// @name Destructors
    /// @{
    /// @brief Resets the class.
    void clear() noexcept;
    /// @brief Destructor.
    ~Parameters();
    /// @}
private:
    class ParametersImpl;
    std::unique_ptr<ParametersImpl> pImpl;    
};
}
#endif
