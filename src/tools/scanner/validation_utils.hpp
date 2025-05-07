#ifndef VALIDATION_UTILS_HPP
#define VALIDATION_UTILS_HPP

#include <set>
#include <string>

namespace FWA::SCANNER {
/**
 * @brief Validates a set of channel numbers for consistency and reasonableness
 *
 * Checks if channel numbers are sequential and within reasonable limits.
 * Reports gaps in the sequence and warns if numbers exceed expected maximums.
 *
 * @param channelNumbers Set of channel numbers to validate
 * @param channelType String describing the type of channels (e.g., "Output",
 * "Input")
 */
void validateChannelNumbers(const std::set<int> &channelNumbers,
                            const std::string &channelType);

} // namespace FWA::SCANNER

#endif // VALIDATION_UTILS_HPP
