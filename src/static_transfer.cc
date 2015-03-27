#include "static_transfer.hh"

#include <stdexcept>
#include <utility>

#include "base/file.hh"

namespace {
const char kConnectionHeader[] = "Connection";
const char kContentLengthHeader[] = "Content-Length";
const char kContentRangeHeader[] = "Content-Range";
const char kContentTypeHeader[] = "Content-Type";
const char kRangeHeader[] = "range";

const char kOctetStreamMimeType[] = "application/octet-stream";
}  // anonymous namespace

namespace koohar {

const StringMap StaticTransfer::mime_types_ = {
  {"css", "text/css"},
  {"webm", "video/webm"},
  {"jpe", "image/jpeg"},
  {"jpg", "image/jpeg"},
  {"ico", "image/x-icon"},
  {"iff", "image/tiff"},
  {"gif", "image/gif"},
  {"htm", "text/html"},
  {"html", "text/html"},
  {"mk3d", "video/x-matroska-3d"},
  {"mka", "audio/x-matroska"},
  {"mkv", "video/x-matroska"},
  {"mov", "video/quicktime"},
  {"mpe", "video/mpeg"},
  {"mpg", "video/mpeg"},
  {"pdf", "application/pdf"},
  {"peg", "image/jpeg"},
  {"png", "image/png"},
  {"rtf", "application/rtf"},
  {"rtx", "text/richtext"},
  {"json", "application/json"},
  {"tif", "image/tiff"},
  {"tml", "text/html"},
  {"txt", "text/plain"},
  {"wav", "application/x-wav"},
  {"zip", "application/zip"},
  {"js", "application/x-javascript"},
};

StaticTransfer::StaticTransfer(Request&& request,
                               Response&& response,
                               const ServerConfig& config)
    : request_(std::move(request)),
      response_(std::move(response)),
      config_(config) {
}

void StaticTransfer::Serve() {
  const std::string static_dir = config_.static_dir();
  std::string file_name = static_dir.empty() ? "." : static_dir;
  file_name.append(request_.uri());

  if (File::IsDirectory(file_name.c_str())) {
    return HandleError(400);
  }

  File static_file(file_name);

  if (IsVulnerable(file_name) ||
      !static_file.Open(File::AccessType::ReadOnly)) {
    return HandleError(404);
  }

  response_.Header(kConnectionHeader, "Close");
  response_.Header(kContentTypeHeader, MimeFromName(file_name));

  unsigned long shift = 0;
  unsigned long size = 0;
  // Check if header "Range: bytes=xxx-yyy" set up.
  if (!request_.Header(kRangeHeader).empty() ||
      static_file.GetSize() > kMaxStaticSize) {
    const std::string range_header = request_.Header(kRangeHeader);
    if (!range_header.empty()) {
      const std::size_t eq_pos = range_header.find("=");
      const std::size_t sep_pos = range_header.find("-");
      if (eq_pos == std::string::npos || sep_pos == std::string::npos ||
          sep_pos < eq_pos) {  // Something really wierd with range.
        return HandleError(416);
      }
      const std::string start =
          range_header.substr(eq_pos + 1, sep_pos - eq_pos - 1);
      const std::string end =
          range_header.substr(sep_pos + 1, range_header.length() - sep_pos);
      if (start.empty()) {
        size = std::atol(end.c_str());
        shift = static_file.GetSize() - size;
      } else {
        shift = std::atol(start.c_str());
        size = end.empty()
          ? (static_file.GetSize() - shift)
          : (std::atol(end.c_str()) - shift);
      }
      if (shift + size > static_file.GetSize()) {
        return HandleError(416);
      }
    }
    size = size ? size : static_file.GetSize();
    size = size > kMaxStaticSize ? kMaxStaticSize : size;
    response_.WriteHead(206);
    std::stringstream range;
    range << "bytes " << shift << "-" << (size + shift - 1)
      << "/" << static_file.GetSize();
    response_.Header(kContentRangeHeader, range.str().c_str());
  } else {
    response_.Header(kContentLengthHeader,
                     std::to_string(static_file.GetSize()));
    response_.WriteHead(200);
  }

  response_.SendFile(static_file.GetHandle(), static_file.GetSize(), 0);
  response_.End();
}

bool StaticTransfer::IsVulnerable(const std::string& file_name) {
  return file_name.find("..") != std::string::npos || file_name[0] == '/';
}

void StaticTransfer::HandleError(const unsigned short Code) {
    const std::string error_page = config_.GetErrorPage(Code);
    error_page.empty() ?
        response_.WriteHead(Code) : response_.Redirect(error_page);
    response_.End();
}

std::string StaticTransfer::MimeFromName(const std::string& file_name) {
  try {
    const std::string& file_extension =
        file_name.substr(file_name.rfind('.') + 1);
    return mime_types_.at(file_extension);
  } catch (std::out_of_range& e) {
    return kOctetStreamMimeType;
  }
}

}  // namespace koohar
