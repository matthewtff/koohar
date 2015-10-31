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
namespace {
// TODO(matthewtff): Check for other kinds of attacks like quering symlink etc.
bool IsVulnerable(const std::string& file_name) {
  return file_name.find("..") != std::string::npos || file_name[0] == '/';
}

std::string MimeFromName(const std::string& file_name) {
  auto file_extension = file_name.substr(file_name.rfind('.') + 1);
  auto mime_type = StaticTransfer::mime_types().find(file_extension);
  return (mime_type == StaticTransfer::mime_types().end()) ?
      kOctetStreamMimeType : mime_type->second;
}

std::string GetRelativePath(const std::string& static_dir,
                            const std::string& uri) {
  std::string file_name = static_dir.empty() ? "." : static_dir;
  file_name.append(uri);
  return file_name;
}

std::string CreateRangeHeader(const unsigned long offset,
                              const unsigned long size,
                              const unsigned long overall_size) {
  std::stringstream range;
  range << "bytes " << offset << "-" << (size + offset - 1) << "/"
      << overall_size;
  return range.str();
}

std::pair<size_t, size_t> ParseRangeHeader(const std::string& range_header) {
  const size_t eq_pos = range_header.find("=");
  const size_t sep_pos = range_header.find("-");
  if (eq_pos == std::string::npos || sep_pos == std::string::npos ||
      sep_pos < eq_pos) {  // Something really wierd with range.
    return std::make_pair(0ul, 0ul);
  }
  const std::string start =
      range_header.substr(eq_pos + 1, sep_pos - eq_pos - 1);
  const std::string end =
      range_header.substr(sep_pos + 1, range_header.length() - sep_pos);
  return std::make_pair<size_t, size_t>(
      std::atol(start.c_str()), std::atol(end.c_str()));
}
}  // anonymous namespace

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
  const std::string& file_name =
      GetRelativePath(config_.static_dir(), request_.uri());
  if (File::IsDirectory(file_name))
    return HandleError(400);

  File file(file_name);

  if (IsVulnerable(file_name) || !file.Open(File::AccessType::ReadOnly))
    return HandleError(404);

  response_.Header(kConnectionHeader, "Close");
  response_.Header(kContentTypeHeader, MimeFromName(file_name));

  // Check if header "Range: bytes=xxx-yyy" set up.
  const std::string& range_header = request_.Header(kRangeHeader);
  if (range_header.empty() && file.GetSize() <= kMaxStaticSize)
    return SendAllFile(file);

  unsigned long shift = 0;
  unsigned long size = 0;
  if (!range_header.empty()) {
    auto range = ParseRangeHeader(range_header);
    if (range.first) {
      shift = range.first;
      size = range.second ? (range.second - shift) : (file.GetSize() - shift);
    } else {
      size = range.second;
      shift = file.GetSize() - size;
    }
    if (shift + size > file.GetSize())
      return HandleError(416);  // "Requested Range Not Satisfiable"
  }
  size = std::min(size ? size : file.GetSize(), kMaxStaticSize);
  response_.WriteHead(206);
  response_.Header(kContentRangeHeader,
                   CreateRangeHeader(shift, size, file.GetSize()));

  response_.SendFile(file.GetHandle(), size, shift);
  response_.End();
}

void StaticTransfer::HandleError(const unsigned short Code) {
    const std::string& error_page = config_.GetErrorPage(Code);
    error_page.empty() ?
        response_.WriteHead(Code) : response_.Redirect(error_page);
    response_.End();
}


void StaticTransfer::SendAllFile(const File& file) {
  response_.Header(kContentLengthHeader, std::to_string(file.GetSize()));
  response_.WriteHead(200);
  response_.SendFile(file.GetHandle(), file.GetSize(), 0u);
  response_.End();
}

}  // namespace koohar
