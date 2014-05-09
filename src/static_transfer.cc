#include "static_transfer.hh"

#include <utility>

#include "base/file.hh"

namespace koohar {

StringMap StaticTransfer::m_mime_types = {
  {"css", "text/css"},
  {"ebm", "video/webm"}, // webm actually
  {"jpe", "image/jpeg"},
  {"jpg", "image/jpeg"},
  {"iff", "image/tiff"},
  {"gif", "image/gif"},
  {"htm", "text/html"},
  {"k3d", "video/x-matroska-3d"}, // mk3d actually
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
  {"son", "application/json"}, // json actually
  {"tif", "image/tiff"},
  {"tml", "text/html"},
  {"txt", "text/plain"},
  {"wav", "application/x-wav"},
  {"zip", "application/zip"},
  {".js", "application/x-javascript"},
};

StaticTransfer::StaticTransfer(const Request& request,
                               Response&& response,
                               const ServerConfig& config)
    : m_request(request),
      m_response(std::forward<Response>(response)),
      m_config(config) {
}

void StaticTransfer::Serve() {
  const std::string static_dir = m_config.getStaticDir();
  std::string file_name = static_dir.empty() ? "." : static_dir;
  file_name.append(m_request.uri());

  if (File::isDirectory(file_name.c_str()))
    return handleError(400);

  File static_file(file_name);

  if (isVulnerable(file_name) || !static_file.open(File::AccessType::ReadOnly))
    return handleError(404);

  m_response.header("Connection", "Close");
  m_response.header("Content-Type", mimeFromName(file_name));

  unsigned long shift = 0;
  unsigned long size = 0;
  // Check if header "Range: bytes=xxx-yyy" set up.
  if (!m_request.header("range").empty()
    || static_file.getSize() > MaxStaticSize)
  {
    std::string range_header = m_request.header("range");
    if (!range_header.empty()) {
      const std::size_t eq_pos = range_header.find("=");
      const std::size_t sep_pos = range_header.find("-");
      if (eq_pos == range_header.npos || sep_pos == range_header.npos
        || sep_pos < eq_pos)
      { // Something really wierd with range.
        const std::string error_page = m_config.getErrorPage(416);
        error_page.empty()
            ? m_response.writeHead(416) : m_response.redirect(error_page);
        m_response.end();
        return;
      }
      const std::string start =
          range_header.substr(eq_pos + 1, sep_pos - eq_pos - 1);
      const std::string end =
          range_header.substr(sep_pos + 1, range_header.length() - sep_pos);
      if (start.empty()) {
        size = std::atol(end.c_str());
        shift = static_file.getSize() - size;
      } else {
        shift = std::atol(start.c_str());
        size = end.empty()
          ? (static_file.getSize() - shift)
          : (std::atol(end.c_str()) - shift);
      }
      if (shift + size > static_file.getSize()) {
        const std::string error_page = m_config.getErrorPage(416);
        error_page.empty()
            ? m_response.writeHead(416) : m_response.redirect(error_page);
        m_response.end();
        return;
      }
    }
    size = size ? size : static_file.getSize();
    size = size > MaxStaticSize ? MaxStaticSize : size;
    m_response.writeHead(206);
    std::stringstream range;
    range << "bytes " << shift << "-" << (size + shift - 1)
      << "/" << static_file.getSize();
    m_response.header("Content-Range", range.str().c_str());
  } else {
    std::stringstream length;
    length << static_file.getSize();
    m_response.header("Content-Length", length.str());
    m_response.writeHead(200);
  }

  m_response.sendFile(static_file.getHandle(), static_file.getSize(), 0);
  m_response.end();

}

bool StaticTransfer::isVulnerable (const std::string& FileName) {
  return FileName.find("..") != FileName.npos || FileName[0] == '/';
}

void StaticTransfer::handleError (const unsigned short Code) {
    const std::string error_page = m_config.getErrorPage(Code);
    error_page.empty()
        ? m_response.writeHead(Code) : m_response.redirect(error_page);
    m_response.end();
}

std::string StaticTransfer::mimeFromName (const std::string& FileName) {
  //TODO(matthewtff): Use rfind to locate a dot and get correct extension.
  static const std::size_t mime_size = 3;
  std::string mime_substring =
    FileName.substr(FileName.length() - mime_size, mime_size);
  return m_mime_types[mime_substring];
}

}  // namespace koohar
