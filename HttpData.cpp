#include "HttpData.h"
#include "time.h"
#include "Channel.h"
#include "Util.h"
#include "EventLoop.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <iostream>
#include <algorithm>
#include <string.h>
using namespace std;

pthread_once_t MimeType::once_control = PTHREAD_ONCE_INIT;
std::unordered_map<std::string, std::string> MimeType::mime;


const __uint32_t DEFAULT_EVENT = EPOLLIN | EPOLLET| EPOLLONESHOT;
const int DEFAULT_EXPIRED_TIME = 2000; // ms
const int DEFAULT_KEEP_ALIVE_TIME = 5 * 60 * 1000; // ms

char favicon[1151] = {
'\x00','\x00','\x01','\x00','\x01','\x00','\x10','\x10','\x00','\x00',
'\x01','\x00','\x20','\x00','\x68','\x04','\x00','\x00','\x16','\x00',
'\x00','\x00','\x28','\x00','\x00','\x00','\x10','\x00','\x00','\x00',
'\x20','\x00','\x00','\x00','\x01','\x00','\x20','\x00','\x00','\x00',
'\x00','\x00','\x00','\x04','\x00','\x00','\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00',
'\x1c','\xd0','\xf3','\x04','\x24','\xac','\xe5','\x19','\x13','\xd6',
'\xf6','\x9a','\x1b','\xed','\xff','\xb9','\x0f','\x7d','\xa0','\x26',
'\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00',
'\x00','\x00','\x19','\xf3','\xff','\x94','\x16','\xe6','\xff','\xb5',
'\x17','\xe6','\xfe','\x20','\x00','\x00','\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00',
'\x1f','\xc4','\xf1','\xa7','\x0e','\xe7','\xfe','\xf5','\x15','\xd7',
'\xfc','\xfb','\x27','\xd3','\xff','\xff','\x32','\xe5','\xff','\xff',
'\x31','\x72','\x9d','\xec','\x56','\x4a','\x49','\xd2','\x55','\x56',
'\x59','\xd8','\x37','\x3a','\x44','\xde','\x1c','\xc1','\xe6','\xff',
'\x14','\xe5','\xff','\xff','\x11','\xe7','\xff','\xf5','\x19','\xdf',
'\xfb','\x48','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00','\x2e','\xab','\xf5','\xba','\x31','\xc2',
'\xff','\xff','\x60','\xc1','\xff','\xff','\x77','\xc7','\xff','\xff',
'\x41','\x8e','\xdd','\xff','\xae','\xbf','\xd0','\xff','\xff','\xff',
'\xff','\xff','\xff','\xff','\xff','\xff','\xf6','\xf3','\xf6','\xff',
'\x42','\xa5','\xef','\xff','\x30','\xbe','\xff','\xff','\x28','\xce',
'\xff','\xff','\x14','\xe0','\xff','\xff','\x21','\xc9','\xf6','\xb1',
'\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x37','\x91',
'\xf0','\x99','\x72','\xbc','\xfe','\xff','\xbe','\xea','\xff','\xff',
'\x86','\xd4','\xff','\xff','\x00','\x00','\x1c','\xff','\x90','\x91',
'\x8e','\xff','\xee','\xff','\xff','\xff','\xde','\xec','\xf9','\xff',
'\xec','\xf4','\xfc','\xff','\x6e','\xb8','\xff','\xff','\x6f','\xc2',
'\xff','\xff','\x6d','\xd7','\xff','\xff','\x2f','\xca','\xff','\xff',
'\x36','\x94','\xec','\x59','\x00','\x00','\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00','\x00','\x00','\x49','\xa4','\xff','\x4d',
'\x99','\xd5','\xee','\xf6','\x43','\x75','\x9d','\xff','\x71','\x71',
'\x74','\xff','\xe5','\xf1','\xfd','\xff','\xcb','\xd8','\xe5','\xff',
'\xcb','\xd8','\xe2','\xff','\xdc','\xe2','\xe6','\xff','\x81','\xbb',
'\xf2','\xff','\x2d','\x45','\x5c','\xff','\x40','\x49','\x50','\xff',
'\x35','\x76','\xab','\x94','\x00','\x00','\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00','\x01','\x00','\x00','\xb0','\x66','\x63',
'\x63','\xff','\xe0','\xe6','\xeb','\xff','\xdb','\xe2','\xe7','\xff',
'\xdd','\xe3','\xe7','\xff','\xe0','\xe5','\xe9','\xff','\xe4','\xe9',
'\xea','\xff','\xd2','\xd7','\xdc','\xff','\x42','\x40','\x41','\xff',
'\x1f','\x11','\x07','\xff','\x04','\x00','\x00','\x75','\x00','\x00',
'\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x13','\x0d',
'\x0a','\xa0','\x1e','\x1d','\x1d','\xff','\xdc','\xde','\xe0','\xff',
'\xf5','\xf4','\xf5','\xff','\xf8','\xf8','\xf7','\xff','\xf7','\xf7',
'\xf8','\xff','\xf7','\xf8','\xf9','\xff','\xfd','\xfc','\xfb','\xff',
'\x4d','\x4c','\x4d','\xff','\x00','\x00','\x00','\xff','\x0a','\x07',
'\x06','\x7c','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00',
'\x00','\x00','\x2a','\x2a','\x2a','\x3f','\x1e','\x1d','\x1d','\xff',
'\xb0','\xae','\xad','\xff','\xff','\xff','\xff','\xff','\xfc','\xfb',
'\xfb','\xff','\xfc','\xfb','\xfb','\xff','\xff','\xff','\xff','\xff',
'\xd7','\xd6','\xd6','\xff','\x10','\x11','\x11','\xff','\x11','\x11',
'\x12','\xff','\x16','\x16','\x16','\x2c','\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00',
'\x2c','\x2d','\x2d','\xb1','\x1e','\x1d','\x1d','\xff','\xf8','\xfb',
'\xfd','\xff','\xfa','\xf9','\xf7','\xff','\xf5','\xf4','\xf3','\xff',
'\xfe','\xff','\xff','\xff','\x65','\x66','\x68','\xff','\x03','\x03',
'\x02','\xff','\x2b','\x2b','\x2b','\x9c','\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x04','\x03',
'\x01','\xe6','\xcb','\xc4','\xc1','\xff','\xe9','\xe8','\xec','\xff',
'\xf3','\xec','\xec','\xff','\xee','\xe7','\xe8','\xff','\x07','\x0a',
'\x0b','\xff','\x1b','\x1b','\x1b','\xd4','\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00','\x00','\x4e','\x54','\x86','\xa0','\xff',
'\x25','\xd4','\xff','\xff','\x25','\xd4','\xff','\xff','\x91','\x9d',
'\xa5','\xff','\x00','\x00','\x00','\xf2','\x26','\x27','\x26','\x10',
'\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00','\x00','\x00','\x07','\x1a','\x2d','\x1a',
'\x13','\x8f','\xc6','\xff','\x59','\xf5','\xff','\xff','\x25','\xd4',
'\xff','\xff','\x03','\x57','\x7e','\xff','\x00','\x00','\x00','\xb8',
'\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x21','\x6a','\x90','\xac','\xff','\x6e','\x90',
'\xba','\xff','\x73','\x98','\xbc','\xff','\x5f','\x67','\x6f','\xff',
'\x00','\x00','\x00','\xbc','\x00','\x00','\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x2a','\x8f','\x8b',
'\x88','\xff','\x49','\x3e','\x37','\xff','\xa5','\x9c','\x96','\xff',
'\x3b','\x38','\x37','\xff','\x00','\x00','\x00','\xba','\x00','\x00',
'\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x13','\x13',
'\x13','\x0a','\x48','\x48','\x48','\xf1','\x44','\x44','\x44','\xff',
'\x44','\x44','\x44','\xff','\x44','\x44','\x44','\xff','\x00','\x00',
'\x00','\x76','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00','\x00','\x00','\x41','\x41','\x41','\x36',
'\x90','\x90','\x90','\xc9','\x97','\x97','\x97','\xe8','\x26','\x26',
'\x26','\x97','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00','\x00','\x00','\xc1','\xc7','\x00','\x00',
'\x80','\x03','\x00','\x00','\x80','\x01','\x00','\x00','\x80','\x01',
'\x00','\x00','\xc0','\x03','\x00','\x00','\xe0','\x03','\x00','\x00',
'\xe0','\x03','\x00','\x00','\xe0','\x03','\x00','\x00','\xf0','\x07',
'\x00','\x00','\xf8','\x0f','\x00','\x00','\xf8','\x0f','\x00','\x00',
'\xf8','\x1f','\x00','\x00','\xf8','\x1f','\x00','\x00','\xf8','\x1f',
'\x00','\x00','\xf8','\x1f','\x00','\x00','\xfc','\x3f','\x00','\x00',
'\x0f'
};

void MimeType::init()
{
    mime[".html"] = "text/html";
    mime[".avi"] = "video/x-msvideo";
    mime[".bmp"] = "image/bmp";
    mime[".c"] = "text/plain";
    mime[".doc"] = "application/msword";
    mime[".gif"] = "image/gif";
    mime[".gz"] = "application/x-gzip";
    mime[".htm"] = "text/html";
    mime[".ico"] = "image/x-icon";
    mime[".jpg"] = "image/jpeg";
    mime[".png"] = "image/png";
    mime[".txt"] = "text/plain";
    mime[".mp3"] = "audio/mp3";
    mime["default"] = "text/html";
}

std::string MimeType::getMime(const std::string &suffix)
{
    pthread_once(&once_control, MimeType::init);
    if (mime.find(suffix) == mime.end())
        return mime["default"];
    else
        return mime[suffix];
}


HttpData::HttpData(EventLoop *loop, int connfd):
        loop_(loop),
        channel_(new Channel(loop, connfd)),
        fd_(connfd),
        error_(false),
        connectionState_(H_CONNECTED),
        method_(METHOD_GET),
        HTTPVersion_(HTTP_11),
        nowReadPos_(0), 
        state_(STATE_PARSE_URI), 
        hState_(H_START),
        keepAlive_(false)
{
    //loop_->queueInLoop(bind(&HttpData::setHandlers, this));
    channel_->setReadHandler(bind(&HttpData::handleRead, this));
    channel_->setWriteHandler(bind(&HttpData::handleWrite, this));
    channel_->setConnHandler(bind(&HttpData::handleConn, this));
}

void HttpData::reset()
{
    //inBuffer_.clear();
    fileName_.clear();
    path_.clear();
    nowReadPos_ = 0;
    state_ = STATE_PARSE_URI;
    hState_ = H_START;
    headers_.clear();
    //keepAlive_ = false;
    if (timer_.lock())
    {
        shared_ptr<TimerNode> my_timer(timer_.lock());
        my_timer->clearReq();
        timer_.reset();
    }
}

void HttpData::seperateTimer()
{
    //cout << "seperateTimer" << endl;
    if (timer_.lock())
    {
        shared_ptr<TimerNode> my_timer(timer_.lock());
        my_timer->clearReq();
        timer_.reset();
    }
}

void HttpData::handleRead()
{
    __uint32_t &events_ = channel_->getEvents();
    do
    {
        bool zero = false;
        int read_num = readn(fd_, inBuffer_, zero);
        cout<< "Request: " << inBuffer_;
        if (connectionState_ == H_DISCONNECTING)
        {
            inBuffer_.clear();
            break;
        }
        //cout << inBuffer_ << endl;
        if (read_num < 0)
        {
            perror("1");
            error_ = true;
            handleError(fd_, 400, "Bad Request");
            break;
        }
        // else if (read_num == 0)
        // {
        //     error_ = true;
        //     break; 
        // }
        else if (zero)
        {
            // 有请求出现但是读不到数据，可能是Request Aborted，或者来自网络的数据没有达到等原因
            // 最可能是对端已经关闭了，统一按照对端已经关闭处理
            //error_ = true;
            connectionState_ = H_DISCONNECTING;
            if (read_num == 0)
            {
                //error_ = true;
                break;
            }
            //cout << "readnum == 0" << endl;
        }


        if (state_ == STATE_PARSE_URI)
        {
            URIState flag = this->parseURI();
            if (flag == PARSE_URI_AGAIN)
                break;
            else if (flag == PARSE_URI_ERROR)
            {
                perror("2");
                cout<< "FD = " << fd_ << "," << inBuffer_ << "******";
                inBuffer_.clear();
                error_ = true;
                handleError(fd_, 400, "Bad Request");
                break;
            }
            else
                state_ = STATE_PARSE_HEADERS;
        }
        if (state_ == STATE_PARSE_HEADERS)
        {
            HeaderState flag = this->parseHeaders();
            if (flag == PARSE_HEADER_AGAIN)
                break;
            else if (flag == PARSE_HEADER_ERROR)
            {
                perror("3");
                error_ = true;
                handleError(fd_, 400, "Bad Request");
                break;
            }
            if(method_ == METHOD_POST)
            {
                // POST方法准备
                state_ = STATE_RECV_BODY;
            }
            else 
            {
                state_ = STATE_ANALYSIS;
            }
        }
        if (state_ == STATE_RECV_BODY)
        {
            int content_length = -1;
            if (headers_.find("Content-length") != headers_.end())
            {
                content_length = stoi(headers_["Content-length"]);
            }
            else
            {
                //cout << "(state_ == STATE_RECV_BODY)" << endl;
                error_ = true;
                handleError(fd_, 400, "Bad Request: Lack of argument (Content-length)");
                break;
            }
            if (static_cast<int>(inBuffer_.size()) < content_length)
                break;
            state_ = STATE_ANALYSIS;
        }
        if (state_ == STATE_ANALYSIS)
        {
            AnalysisState flag = this->analysisRequest();
            if (flag == ANALYSIS_SUCCESS)
            {
                state_ = STATE_FINISH;
                break;
            }
            else
            {
                //cout << "state_ == STATE_ANALYSIS" << endl;
                error_ = true;
                break;
            }
        }
    } while (false);
    //cout << "state_=" << state_ << endl;
    if (!error_)
    {
        if (outBuffer_.size() > 0)
        {
            handleWrite();
            //events_ |= EPOLLOUT;
        }
        // error_ may change
        if (!error_ && state_ == STATE_FINISH)
        {
            this->reset();
            if (inBuffer_.size() > 0)
            {
                if (connectionState_ != H_DISCONNECTING)
                    handleRead();
            }

            // if ((keepAlive_ || inBuffer_.size() > 0) && connectionState_ == H_CONNECTED)
            // {
            //     this->reset();
            //     events_ |= EPOLLIN;
            // }
        }
        else if (!error_ && connectionState_ != H_DISCONNECTED)
            events_ |= EPOLLIN;
    }
}

void HttpData::handleWrite()
{
    if (!error_ && connectionState_ != H_DISCONNECTED)
    {
        __uint32_t &events_ = channel_->getEvents();
        if (writen(fd_, outBuffer_) < 0)
        {
            perror("writen");
            events_ = 0;
            error_ = true;
        }
        if (outBuffer_.size() > 0)
            events_ |= EPOLLOUT;
    }
}

void HttpData::handleConn()
{
    seperateTimer();
    __uint32_t &events_ = channel_->getEvents();
    if (!error_ && connectionState_ == H_CONNECTED)
    {
        if (events_ != 0)
        {
            int timeout = DEFAULT_EXPIRED_TIME;
            if (keepAlive_)
                timeout = DEFAULT_KEEP_ALIVE_TIME;
            if ((events_ & EPOLLIN) && (events_ & EPOLLOUT))
            {
                events_ = __uint32_t(0);
                events_ |= EPOLLOUT;
            }
            //events_ |= (EPOLLET | EPOLLONESHOT);
            events_ |= EPOLLET;
            loop_->updatePoller(channel_, timeout);

        }
        else if (keepAlive_)
        {
            events_ |= (EPOLLIN | EPOLLET);
            //events_ |= (EPOLLIN | EPOLLET | EPOLLONESHOT);
            int timeout = DEFAULT_KEEP_ALIVE_TIME;
            loop_->updatePoller(channel_, timeout);
        }
        else
        {
            //cout << "close normally" << endl;
            // loop_->shutdown(channel_);
            // loop_->runInLoop(bind(&HttpData::handleClose, shared_from_this()));
            events_ |= (EPOLLIN | EPOLLET);
            //events_ |= (EPOLLIN | EPOLLET | EPOLLONESHOT);
            int timeout = (DEFAULT_KEEP_ALIVE_TIME >> 1);
            loop_->updatePoller(channel_, timeout);
        }
    }
    else if (!error_ && connectionState_ == H_DISCONNECTING && (events_ & EPOLLOUT))
    {
        events_ = (EPOLLOUT | EPOLLET);
    }
    else
    {
        //cout << "close with errors" << endl;
        loop_->runInLoop(bind(&HttpData::handleClose, shared_from_this()));
    }
}


URIState HttpData::parseURI()
{
    string &str = inBuffer_;
    string cop = str;
    // 读到完整的请求行再开始解析请求
    size_t pos = str.find('\r', nowReadPos_);
    if (pos < 0)
    {
        return PARSE_URI_AGAIN;
    }
    // 去掉请求行所占的空间，节省空间
    string request_line = str.substr(0, pos);
    if (str.size() > pos + 1)
        str = str.substr(pos + 1);
    else 
        str.clear();
    // Method
    int posGet = request_line.find("GET");
    int posPost = request_line.find("POST");
    int posHead = request_line.find("HEAD");

    if (posGet >= 0)
    {
        pos = posGet;
        method_ = METHOD_GET;
    }
    else if (posPost >= 0)
    {
        pos = posPost;
        method_ = METHOD_POST;
    }
    else if (posHead >= 0)
    {
        pos = posHead;
        method_ = METHOD_HEAD;
    }
    else
    {
        return PARSE_URI_ERROR;
    }

    // filename
    pos = request_line.find("/", pos);
    if (pos < 0)
    {
        fileName_ = "index.html";
        HTTPVersion_ = HTTP_11;
        return PARSE_URI_SUCCESS;
    }
    else
    {
        size_t _pos = request_line.find(' ', pos);
        if (_pos < 0)
            return PARSE_URI_ERROR;
        else
        {
            if (_pos - pos > 1)
            {
                fileName_ = request_line.substr(pos + 1, _pos - pos - 1);
                size_t __pos = fileName_.find('?');
                if (__pos >= 0)
                {
                    fileName_ = fileName_.substr(0, __pos);
                }
            }
                
            else
                fileName_ = "index.html";
        }
        pos = _pos;
    }
    //cout << "fileName_: " << fileName_ << endl;
    // HTTP 版本号
    pos = request_line.find("/", pos);
    if (pos < 0)
        return PARSE_URI_ERROR;
    else
    {
        if (request_line.size() - pos <= 3)
            return PARSE_URI_ERROR;
        else
        {
            string ver = request_line.substr(pos + 1, 3);
            if (ver == "1.0")
                HTTPVersion_ = HTTP_10;
            else if (ver == "1.1")
                HTTPVersion_ = HTTP_11;
            else
                return PARSE_URI_ERROR;
        }
    }
    return PARSE_URI_SUCCESS;
}

HeaderState HttpData::parseHeaders()
{
    string &str = inBuffer_;
    int key_start = -1, key_end = -1, value_start = -1, value_end = -1;
    int now_read_line_begin = 0;
    bool notFinish = true;
    size_t i = 0;
    for (; i < str.size() && notFinish; ++i)
    {
        switch(hState_)
        {
            case H_START:
            {
                if (str[i] == '\n' || str[i] == '\r')
                    break;
                hState_ = H_KEY;
                key_start = i;
                now_read_line_begin = i;
                break;
            }
            case H_KEY:
            {
                if (str[i] == ':')
                {
                    key_end = i;
                    if (key_end - key_start <= 0)
                        return PARSE_HEADER_ERROR;
                    hState_ = H_COLON;
                }
                else if (str[i] == '\n' || str[i] == '\r')
                    return PARSE_HEADER_ERROR;
                break;  
            }
            case H_COLON:
            {
                if (str[i] == ' ')
                {
                    hState_ = H_SPACES_AFTER_COLON;
                }
                else
                    return PARSE_HEADER_ERROR;
                break;  
            }
            case H_SPACES_AFTER_COLON:
            {
                hState_ = H_VALUE;
                value_start = i;
                break;  
            }
            case H_VALUE:
            {
                if (str[i] == '\r')
                {
                    hState_ = H_CR;
                    value_end = i;
                    if (value_end - value_start <= 0)
                        return PARSE_HEADER_ERROR;
                }
                else if (i - value_start > 255)
                    return PARSE_HEADER_ERROR;
                break;  
            }
            case H_CR:
            {
                if (str[i] == '\n')
                {
                    hState_ = H_LF;
                    string key(str.begin() + key_start, str.begin() + key_end);
                    string value(str.begin() + value_start, str.begin() + value_end);
                    headers_[key] = value;
                    now_read_line_begin = i;
                }
                else
                    return PARSE_HEADER_ERROR;
                break;  
            }
            case H_LF:
            {
                if (str[i] == '\r')
                {
                    hState_ = H_END_CR;
                }
                else
                {
                    key_start = i;
                    hState_ = H_KEY;
                }
                break;
            }
            case H_END_CR:
            {
                if (str[i] == '\n')
                {
                    hState_ = H_END_LF;
                }
                else
                    return PARSE_HEADER_ERROR;
                break;
            }
            case H_END_LF:
            {
                notFinish = false;
                key_start = i;
                now_read_line_begin = i;
                break;
            }
        }
    }
    if (hState_ == H_END_LF)
    {
        str = str.substr(i);
        return PARSE_HEADER_SUCCESS;
    }
    str = str.substr(now_read_line_begin);
    return PARSE_HEADER_AGAIN;
}

AnalysisState HttpData::analysisRequest()
{
    if (method_ == METHOD_POST)
    {
        // ------------------------------------------------------
        // My CV stitching handler which requires OpenCV library
        // ------------------------------------------------------
        // string header;
        // header += string("HTTP/1.1 200 OK\r\n");
        // if(headers_.find("Connection") != headers_.end() && headers_["Connection"] == "Keep-Alive")
        // {
        //     keepAlive_ = true;
        //     header += string("Connection: Keep-Alive\r\n") + "Keep-Alive: timeout=" + to_string(DEFAULT_KEEP_ALIVE_TIME) + "\r\n";
        // }
        // int length = stoi(headers_["Content-length"]);
        // vector<char> data(inBuffer_.begin(), inBuffer_.begin() + length);
        // Mat src = imdecode(data, CV_LOAD_IMAGE_ANYDEPTH|CV_LOAD_IMAGE_ANYCOLOR);
        // //imwrite("receive.bmp", src);
        // Mat res = stitch(src);
        // vector<uchar> data_encode;
        // imencode(".png", res, data_encode);
        // header += string("Content-length: ") + to_string(data_encode.size()) + "\r\n\r\n";
        // outBuffer_ += header + string(data_encode.begin(), data_encode.end());
        // inBuffer_ = inBuffer_.substr(length);
        // return ANALYSIS_SUCCESS;
    }
    else if (method_ == METHOD_GET || method_ == METHOD_HEAD)
    {
        string header;
        header += "HTTP/1.1 200 OK\r\n";
        if(headers_.find("Connection") != headers_.end() && (headers_["Connection"] == "Keep-Alive" || headers_["Connection"] == "keep-alive"))
        {
            keepAlive_ = true;
            header += string("Connection: Keep-Alive\r\n") + "Keep-Alive: timeout=" + to_string(DEFAULT_KEEP_ALIVE_TIME) + "\r\n";
        }
        reverse(fileName_.begin(),fileName_.end());
        int redot_pos = fileName_.find('.');
        int dot_pos = fileName_.length() - redot_pos - 1;

        string filetype;
        reverse(fileName_.begin(),fileName_.end());
        if (dot_pos < 0) 
            filetype = MimeType::getMime("default");
        else
            filetype = MimeType::getMime(fileName_.substr(dot_pos));
        

        // echo test
        if (fileName_ == "hello")
        {
            string hello_;
            hello_ +="#Good night#";
            outBuffer_ = "HTTP/1.1 200 OK\r\nContent-type: text/plain\r\nContent-Length: " + to_string(hello_.length()) + "\r\n\r\n";
            outBuffer_ += hello_; 
            return ANALYSIS_SUCCESS;
        }
        if (fileName_ == "favicon.ico")
        {
            header += "Content-Type: image/png\r\n";
            header += "Content-Length: " + to_string(sizeof favicon) + "\r\n";
            header += "Server: Micro Web Server\r\n";

            header += "\r\n";
            outBuffer_ += header;
            outBuffer_ += string(favicon, favicon + sizeof favicon);;
            return ANALYSIS_SUCCESS;
        }

        struct stat sbuf;
        if (stat(fileName_.c_str(), &sbuf) < 0)
        {
            header.clear();
            handleError(fd_, 404, "Not Found!");
            return ANALYSIS_ERROR;
        }
        header += "Content-Type: " + filetype + "\r\n";
        header += "Content-Length: " + to_string(sbuf.st_size) + "\r\n";
        header += "Server: Micro Web Server\r\n";
        // 头部结束
        header += "\r\n";
        outBuffer_ += header;
        
        if (method_ == METHOD_HEAD)
            return ANALYSIS_SUCCESS;

        int src_fd = open(fileName_.c_str(), O_RDONLY, 0);
        if (src_fd < 0)
        {
          outBuffer_.clear();
          handleError(fd_, 404, "Not Found!");
          return ANALYSIS_ERROR;
        }
		void *mmapRet = mmap(NULL, sbuf.st_size, PROT_READ, MAP_PRIVATE, src_fd, 0);
		close(src_fd);
		if (mmapRet == (void *)-1)
		{
			munmap(mmapRet, sbuf.st_size);
			outBuffer_.clear();
			handleError(fd_, 404, "Not Found!");
			return ANALYSIS_ERROR;
		}
        char *src_addr = static_cast<char*>(mmapRet);
        outBuffer_ += string(src_addr, src_addr + sbuf.st_size);;
        munmap(mmapRet, sbuf.st_size);
        return ANALYSIS_SUCCESS;
    }
    return ANALYSIS_ERROR;
}

void HttpData::handleError(int fd, int err_num, string short_msg)
{
    short_msg = " " + short_msg;
    char send_buff[4096];
    string body_buff, header_buff;
    body_buff += "<html><title>哎~出错了</title>";
    body_buff += "<body bgcolor=\"ffffff\">";
    body_buff += to_string(err_num) + short_msg;
    body_buff += "<hr><em> Mirco Web Server</em>\n</body></html>";

    header_buff += "HTTP/1.1 " + to_string(err_num) + short_msg + "\r\n";
    header_buff += "Content-Type: text/html\r\n";
    header_buff += "Connection: Close\r\n";
    header_buff += "Content-Length: " + to_string(body_buff.size()) + "\r\n";
    header_buff += "Server: Micro Web Server\r\n";;
    header_buff += "\r\n";
    // 错误处理不考虑writen不完的情况
    sprintf(send_buff, "%s", header_buff.c_str());
    writen(fd, send_buff, strlen(send_buff));
    sprintf(send_buff, "%s", body_buff.c_str());
    writen(fd, send_buff, strlen(send_buff));
}

void HttpData::handleClose()
{
    connectionState_ = H_DISCONNECTED;
    shared_ptr<HttpData> guard(shared_from_this());
    loop_->removeFromPoller(channel_);
}


void HttpData::newEvent()
{
    channel_->setEvents(DEFAULT_EVENT);
    loop_->addToPoller(channel_, DEFAULT_EXPIRED_TIME);
}
