#ifndef _XMUTIL_POWERSIM_SIPREADER_H
#define _XMUTIL_POWERSIM_SIPREADER_H

#include <cstddef>
#include <string>
#include <vector>

namespace powersim {

// Lowers a Powersim Studio project (.sip) to an XMILE 1.0 document, which the
// caller then feeds through the ordinary XMILE reader. Covers the model
// (variables, stocks and their flows, equations, graphs, time settings) and
// its diagrams (positions, pipes, connectors, label sides); a diagram that
// does not decode is left out with a note rather than failing. Returns false
// with reasons in errs when the file is not a .sip project or its model uses a
// construct the decoder does not understand -- an unknown construct is always
// an error rather than a silent drop, since the format is reverse-engineered.
// errs may also carry advisory notes on success.
bool SipToXmile(const char *data, size_t len, std::string &xmile, std::vector<std::string> &errs);

// True when data starts with the compound-file signature .sip files use.
bool LooksLikeSip(const char *data, size_t len);

}  // namespace powersim

#endif
