/* Copyright (c) 2014, Fengping Bao <jamol@live.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef __HttpParserImpl_H__
#define __HttpParserImpl_H__

#include "kmdefs.h"
#include "util/util.h"
#include <string>
#include <map>
#include <vector>
#include <functional>

KUMA_NS_BEGIN

class HttpParserImpl
{
public:
    typedef std::function<void(const char*, uint32_t)> DataCallback;
    typedef std::function<void(HttpEvent)> EventCallback;
    typedef std::function<void(const std::string& name, const std::string& value)> EnumrateCallback;
    
    struct CaseIgnoreLess : public std::binary_function<std::string, std::string, bool> {
        bool operator()(const std::string &lhs, const std::string &rhs) const {
            return strcasecmp(lhs.c_str(), rhs.c_str()) < 0;
        }
    };
    typedef std::map<std::string, std::string, CaseIgnoreLess> STRING_MAP;
    
    HttpParserImpl();
    HttpParserImpl(const HttpParserImpl& other);
    HttpParserImpl(HttpParserImpl&& other);
    ~HttpParserImpl();
    HttpParserImpl& operator=(const HttpParserImpl& other);
    HttpParserImpl& operator=(HttpParserImpl&& other);
    
    // return bytes parsed
    int parse(const char* data, uint32_t len);
    void pause();
    void resume();
    // true - http completed
    bool setEOF();
    void reset();
    
    bool isRequest() { return is_request_; }
    bool headerComplete() { return header_complete_; }
    bool complete();
    bool error();
    bool paused() { return paused_; }
    
    int getStatusCode() { return status_code_; }
    const std::string& getLocation() { return getHeaderValue("Location"); }
    const std::string& getUrl() { return url_; }
    const std::string& getUrlPath() { return url_path_; }
    const std::string& getMethod() { return method_; }
    const std::string& getVersion() { return version_; }
    const std::string& getParamValue(const std::string& name);
    const std::string& getHeaderValue(const std::string& name);
    
    void forEachParam(EnumrateCallback& cb);
    void forEachHeader(EnumrateCallback& cb);
    void forEachParam(EnumrateCallback&& cb);
    void forEachHeader(EnumrateCallback&& cb);
    
    void setDataCallback(DataCallback& cb) { cb_data_ = cb; }
    void setEventCallback(EventCallback& cb) { cb_event_ = cb; }
    void setDataCallback(DataCallback&& cb) { cb_data_ = std::move(cb); }
    void setEventCallback(EventCallback&& cb) { cb_event_ = std::move(cb); }
    
private:
    typedef enum{
        PARSE_STATE_CONTINUE,
        PARSE_STATE_DONE,
        PARSE_STATE_ERROR,
        PARSE_STATE_DESTROY
    }ParseState;
    
private:
    ParseState parseHttp(const char*& cur_pos, const char* end);
    bool parseStartLine(const char* line, const char* line_end);
    bool parseHeaderLine(const char* line, const char* line_end);
    ParseState parseChunk(const char*& cur_pos, const char* end);
    bool getLine(const char*& cur_pos, const char* end, const char*& line, const char*& line_end);
    
    bool decodeUrl();
    bool parseUrl();
    void addParamValue(const std::string& name, const std::string& value);
    void addHeaderValue(std::string& name, std::string& value);
    
    bool hasBody();
    bool readEOF();
    
    void onHeaderComplete();
    void onComplete();
    
    int saveData(const char* cur_pos, const char* end);
    bool bufferEmpty() { return str_buf_.empty(); };
    void clearBuffer() { str_buf_.clear(); }
    
private:
    DataCallback        cb_data_;
    EventCallback       cb_event_;
    bool                is_request_;
    
    std::string         str_buf_;
    
    int                 read_state_;
    bool                header_complete_;
    bool                upgrade_;
    bool                paused_;
    
    bool                has_content_length_;
    uint32_t            content_length_;
    
    bool                is_chunked_;
    uint32_t            chunk_state_;
    uint32_t            chunk_size_;
    uint32_t            chunk_bytes_read_;
    
    uint32_t            total_bytes_read_;
    
    // request
    std::string         method_;
    std::string         url_;
    std::string         version_;
    std::string         url_path_;
    STRING_MAP          param_map_;
    STRING_MAP          header_map_;
    
    // response
    int                 status_code_;
    
    bool*               destroy_flag_ptr_;
};

KUMA_NS_END

#endif