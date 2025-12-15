#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#import "NativeMediaCtrl.h"
#include <boost/log/trivial.hpp>

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import <AVKit/AVKit.h>


@interface NativeMediaCtrlObserver : NSObject
{
    Slic3r::GUI::NativeMediaCtrl::Impl* _impl;
}
- (instancetype)initWithImpl:(Slic3r::GUI::NativeMediaCtrl::Impl*)impl;
- (void)observeValueForKeyPath:(NSString*)keyPath
                      ofObject:(id)object
                        change:(NSDictionary<NSKeyValueChangeKey, id>*)change
                       context:(void*)context;
@end

namespace Slic3r { namespace GUI {

class NativeMediaCtrl::Impl
{
public:
    Impl(NativeMediaCtrl* owner, NSView* view);
    ~Impl();

    bool Load(const wxString& url);
    void Play();
    void Stop();
    void Pause();

    NativeMediaState GetState() const { return m_state; }
    wxSize GetVideoSize() const;
    NativeMediaError GetLastError() const { return m_error; }

    void UpdateLayout(int width, int height);
    void OnPlayerStatusChanged(AVPlayerItemStatus status);
    void OnPlayerError(NSError* error);

private:
    void SetupPlayer(NSURL* url);
    void CleanupPlayer();
    void NotifyStateChanged();

    NativeMediaCtrl* m_owner;
    NSView* m_view;
    AVPlayer* m_player;
    AVPlayerLayer* m_player_layer;
    NativeMediaCtrlObserver* m_observer;
    id m_error_observer;
    id m_end_observer;

    NativeMediaState m_state;
    NativeMediaError m_error;
    wxSize m_video_size;
};

NativeMediaCtrl::Impl::Impl(NativeMediaCtrl* owner, NSView* view)
    : m_owner(owner)
    , m_view(view)
    , m_player(nil)
    , m_player_layer(nil)
    , m_observer(nil)
    , m_error_observer(nil)
    , m_end_observer(nil)
    , m_state(NativeMediaState::Stopped)
    , m_error(NativeMediaError::None)
    , m_video_size(1920, 1080)
{
    m_view.wantsLayer = YES;
    m_view.layer = [[CALayer alloc] init];
    CGColorRef color = CGColorCreateGenericRGB(0, 0, 0, 1.0f);
    m_view.layer.backgroundColor = color;
    CGColorRelease(color);
}

NativeMediaCtrl::Impl::~Impl()
{
    CleanupPlayer();
}

void NativeMediaCtrl::Impl::CleanupPlayer()
{
    if (m_observer && m_player) {
        @try {
            [m_player removeObserver:m_observer forKeyPath:@"currentItem.status"];
        } @catch (NSException* e) {
        }
    }

    if (m_error_observer) {
        [[NSNotificationCenter defaultCenter] removeObserver:m_error_observer];
        m_error_observer = nil;
    }

    if (m_end_observer) {
        [[NSNotificationCenter defaultCenter] removeObserver:m_end_observer];
        m_end_observer = nil;
    }

    if (m_player_layer) {
        [m_player_layer removeFromSuperlayer];
        m_player_layer = nil;
    }

    if (m_player) {
        [m_player pause];
        m_player = nil;
    }

    m_observer = nil;
}

bool NativeMediaCtrl::Impl::Load(const wxString& url)
{
    CleanupPlayer();
    m_error = NativeMediaError::None;
    m_state = NativeMediaState::Loading;
    NotifyStateChanged();

    NSString* urlString = [NSString stringWithUTF8String:url.ToUTF8().data()];
    NSURL* nsUrl = [NSURL URLWithString:urlString];

    if (!nsUrl) {
        BOOST_LOG_TRIVIAL(error) << "NativeMediaCtrl_Mac: Invalid URL: " << url.ToStdString();
        m_error = NativeMediaError::StreamNotFound;
        m_state = NativeMediaState::Error;
        NotifyStateChanged();
        return false;
    }

    SetupPlayer(nsUrl);
    return true;
}

void NativeMediaCtrl::Impl::SetupPlayer(NSURL* url)
{
    NSDictionary* options = nil;

    NSString* scheme = [[url scheme] lowercaseString];
    if ([scheme isEqualToString:@"rtsp"] || [scheme isEqualToString:@"rtsps"]) {
        options = @{
            AVURLAssetPreferPreciseDurationAndTimingKey: @NO
        };
    }

    AVURLAsset* asset = [AVURLAsset URLAssetWithURL:url options:options];
    AVPlayerItem* playerItem = [AVPlayerItem playerItemWithAsset:asset];

    m_player = [[AVPlayer alloc] initWithPlayerItem:playerItem];
    m_player.automaticallyWaitsToMinimizeStalling = YES;

    m_player_layer = [AVPlayerLayer playerLayerWithPlayer:m_player];
    m_player_layer.videoGravity = AVLayerVideoGravityResizeAspect;
    m_player_layer.frame = m_view.layer.bounds;
    [m_view.layer addSublayer:m_player_layer];

    m_observer = [[NativeMediaCtrlObserver alloc] initWithImpl:this];

    [m_player addObserver:m_observer
               forKeyPath:@"currentItem.status"
                  options:NSKeyValueObservingOptionNew | NSKeyValueObservingOptionInitial
                  context:nil];

    NativeMediaCtrl::Impl* implPtr = this;

    m_error_observer = [[NSNotificationCenter defaultCenter]
        addObserverForName:AVPlayerItemFailedToPlayToEndTimeNotification
                    object:playerItem
                     queue:[NSOperationQueue mainQueue]
                usingBlock:^(NSNotification* note) {
                    NSError* error = note.userInfo[AVPlayerItemFailedToPlayToEndTimeErrorKey];
                    if (implPtr) {
                        implPtr->OnPlayerError(error);
                    }
                }];

    m_end_observer = [[NSNotificationCenter defaultCenter]
        addObserverForName:AVPlayerItemDidPlayToEndTimeNotification
                    object:playerItem
                     queue:[NSOperationQueue mainQueue]
                usingBlock:^(NSNotification* note) {
                    BOOST_LOG_TRIVIAL(info) << "NativeMediaCtrl_Mac: Playback ended";
                    if (implPtr) {
                        implPtr->m_state = NativeMediaState::Stopped;
                        implPtr->NotifyStateChanged();
                    }
                }];
}

void NativeMediaCtrl::Impl::OnPlayerStatusChanged(AVPlayerItemStatus status)
{
    switch (status) {
        case AVPlayerItemStatusReadyToPlay:
            BOOST_LOG_TRIVIAL(info) << "NativeMediaCtrl_Mac: Player ready to play";
            if (m_state == NativeMediaState::Loading) {
                m_state = NativeMediaState::Playing;
                m_owner->ResetRetryState();
                NotifyStateChanged();
            }
            break;

        case AVPlayerItemStatusFailed:
            BOOST_LOG_TRIVIAL(error) << "NativeMediaCtrl_Mac: Player item failed";
            if (m_player.currentItem.error) {
                OnPlayerError(m_player.currentItem.error);
            } else {
                m_error = NativeMediaError::InternalError;
                m_state = NativeMediaState::Error;
                NotifyStateChanged();
                m_owner->ScheduleRetry();
            }
            break;

        case AVPlayerItemStatusUnknown:
        default:
            break;
    }
}

void NativeMediaCtrl::Impl::Play()
{
    if (m_player) {
        [m_player play];
        if (m_state != NativeMediaState::Loading) {
            m_state = NativeMediaState::Playing;
            NotifyStateChanged();
        }
        BOOST_LOG_TRIVIAL(info) << "NativeMediaCtrl_Mac: Play started";
    }
}

void NativeMediaCtrl::Impl::Stop()
{
    if (m_player) {
        [m_player pause];
        [m_player seekToTime:kCMTimeZero];
    }
    m_state = NativeMediaState::Stopped;
    NotifyStateChanged();
    BOOST_LOG_TRIVIAL(info) << "NativeMediaCtrl_Mac: Stopped";
}

void NativeMediaCtrl::Impl::Pause()
{
    if (m_player) {
        [m_player pause];
        m_state = NativeMediaState::Paused;
        NotifyStateChanged();
        BOOST_LOG_TRIVIAL(info) << "NativeMediaCtrl_Mac: Paused";
    }
}

wxSize NativeMediaCtrl::Impl::GetVideoSize() const
{
    if (m_player && m_player.currentItem) {
        CGSize size = m_player.currentItem.presentationSize;
        if (size.width > 0 && size.height > 0) {
            return wxSize((int)size.width, (int)size.height);
        }
    }
    return m_video_size;
}

void NativeMediaCtrl::Impl::UpdateLayout(int width, int height)
{
    if (m_player_layer) {
        [CATransaction begin];
        [CATransaction setDisableActions:YES];
        m_player_layer.frame = CGRectMake(0, 0, width, height);
        [CATransaction commit];
    }
}

void NativeMediaCtrl::Impl::OnPlayerError(NSError* error)
{
    if (error) {
        BOOST_LOG_TRIVIAL(error) << "NativeMediaCtrl_Mac: Player error: "
                                 << [[error localizedDescription] UTF8String];
    }

    NSInteger code = error ? [error code] : 0;

    if (code == NSURLErrorUserAuthenticationRequired ||
        code == NSURLErrorUserCancelledAuthentication) {
        m_error = NativeMediaError::AuthenticationFailed;
    } else if (code == NSURLErrorCannotFindHost ||
               code == NSURLErrorCannotConnectToHost ||
               code == NSURLErrorNetworkConnectionLost) {
        m_error = NativeMediaError::NetworkUnreachable;
    } else if (code == NSURLErrorTimedOut) {
        m_error = NativeMediaError::ConnectionTimeout;
    } else if (code == NSURLErrorSecureConnectionFailed ||
               code == NSURLErrorServerCertificateHasBadDate ||
               code == NSURLErrorServerCertificateUntrusted) {
        m_error = NativeMediaError::TLSError;
    } else if (code == NSURLErrorFileDoesNotExist ||
               code == NSURLErrorResourceUnavailable) {
        m_error = NativeMediaError::StreamNotFound;
    } else {
        m_error = NativeMediaError::InternalError;
    }

    m_state = NativeMediaState::Error;
    NotifyStateChanged();

    wxCommandEvent event(EVT_NATIVE_MEDIA_ERROR);
    event.SetEventObject(m_owner);
    event.SetInt(static_cast<int>(m_error));
    wxPostEvent(m_owner, event);

    m_owner->ScheduleRetry();
}

void NativeMediaCtrl::Impl::NotifyStateChanged()
{
    wxCommandEvent event(EVT_NATIVE_MEDIA_STATE_CHANGED);
    event.SetEventObject(m_owner);
    event.SetInt(static_cast<int>(m_state));
    wxPostEvent(m_owner, event);
}

NativeMediaCtrl::NativeMediaCtrl(wxWindow* parent)
    : wxWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize)
    , m_retry_enabled(true)
    , m_retry_count(0)
{
    SetBackgroundColour(*wxBLACK);
    m_retry_timer.SetOwner(this);
    Bind(wxEVT_TIMER, &NativeMediaCtrl::OnRetryTimer, this, m_retry_timer.GetId());

    NSView* view = (NSView*)GetHandle();
    m_impl = std::make_unique<Impl>(this, view);
}

NativeMediaCtrl::~NativeMediaCtrl()
{
    m_retry_timer.Stop();
}

bool NativeMediaCtrl::Load(const wxString& url)
{
    if (!IsSupported(url)) {
        BOOST_LOG_TRIVIAL(warning) << "NativeMediaCtrl: Unsupported URL format: " << url.ToStdString();
        return false;
    }

    m_current_url = url;
    ResetRetryState();

    StreamType type = DetectStreamType(url);
    BOOST_LOG_TRIVIAL(info) << "NativeMediaCtrl: Loading " << StreamTypeToString(type).ToStdString()
                            << " stream: " << url.ToStdString();

    return m_impl->Load(url);
}

void NativeMediaCtrl::Play()
{
    if (m_impl) {
        m_impl->Play();
    }
}

void NativeMediaCtrl::Stop()
{
    m_retry_timer.Stop();
    ResetRetryState();
    if (m_impl) {
        m_impl->Stop();
    }
}

void NativeMediaCtrl::Pause()
{
    if (m_impl) {
        m_impl->Pause();
    }
}

NativeMediaState NativeMediaCtrl::GetState() const
{
    return m_impl ? m_impl->GetState() : NativeMediaState::Stopped;
}

wxSize NativeMediaCtrl::GetVideoSize() const
{
    return m_impl ? m_impl->GetVideoSize() : wxSize(1920, 1080);
}

NativeMediaError NativeMediaCtrl::GetLastError() const
{
    return m_impl ? m_impl->GetLastError() : NativeMediaError::None;
}

void NativeMediaCtrl::DoSetSize(int x, int y, int width, int height, int sizeFlags)
{
    wxWindow::DoSetSize(x, y, width, height, sizeFlags);
    if (m_impl && width > 0 && height > 0) {
        m_impl->UpdateLayout(width, height);
    }
}

}} // namespace Slic3r::GUI

@implementation NativeMediaCtrlObserver

- (instancetype)initWithImpl:(Slic3r::GUI::NativeMediaCtrl::Impl*)impl
{
    self = [super init];
    if (self) {
        _impl = impl;
    }
    return self;
}

- (void)observeValueForKeyPath:(NSString*)keyPath
                      ofObject:(id)object
                        change:(NSDictionary<NSKeyValueChangeKey, id>*)change
                       context:(void*)context
{
    if ([keyPath isEqualToString:@"currentItem.status"]) {
        AVPlayer* player = (AVPlayer*)object;
        if (player.currentItem && _impl) {
            dispatch_async(dispatch_get_main_queue(), ^{
                if (_impl) {
                    _impl->OnPlayerStatusChanged(player.currentItem.status);
                }
            });
        }
    }
}

@end
