#include "GpioInput.h"

#ifdef PULSAR_HAS_GPIOD
#include "InputManager.h"

#include <gpiod.h>

namespace
{
constexpr unsigned int kFirstButtonGpio = 17;
constexpr std::size_t kEventBufferCapacity = 16;
constexpr char kConsumerName[] = "pulsar";
}

GpioInput::GpioInput() = default;

GpioInput::~GpioInput()
{
    stop();
}

void GpioInput::start()
{
    m_chip = gpiod_chip_open("/dev/gpiochip0");
    if (!m_chip)
        return;

    gpiod_line_settings *lineSettings = gpiod_line_settings_new();
    gpiod_line_config *lineConfig = gpiod_line_config_new();
    gpiod_request_config *requestConfig = gpiod_request_config_new();

    if (!lineSettings || !lineConfig || !requestConfig
        || gpiod_line_settings_set_direction(lineSettings, GPIOD_LINE_DIRECTION_INPUT) < 0
        || gpiod_line_settings_set_bias(lineSettings, GPIOD_LINE_BIAS_PULL_UP) < 0
        || gpiod_line_settings_set_edge_detection(lineSettings, GPIOD_LINE_EDGE_BOTH) < 0
        || gpiod_line_config_add_line_settings(lineConfig, &kFirstButtonGpio, 1, lineSettings) < 0) {
        gpiod_request_config_free(requestConfig);
        gpiod_line_config_free(lineConfig);
        gpiod_line_settings_free(lineSettings);
        stop();
        return;
    }

    gpiod_request_config_set_consumer(requestConfig, kConsumerName);
    m_request = gpiod_chip_request_lines(m_chip, requestConfig, lineConfig);

    gpiod_request_config_free(requestConfig);
    gpiod_line_config_free(lineConfig);
    gpiod_line_settings_free(lineSettings);

    if (!m_request) {
        stop();
        return;
    }

    m_eventBuffer = gpiod_edge_event_buffer_new(kEventBufferCapacity);
    if (!m_eventBuffer) {
        stop();
        return;
    }

    const gpiod_line_value value = gpiod_line_request_get_value(m_request, kFirstButtonGpio);
    if (value != GPIOD_LINE_VALUE_ERROR)
        InputManager::setButton(InputManager::Button::A, value == GPIOD_LINE_VALUE_INACTIVE);

    const int eventFd = gpiod_line_request_get_fd(m_request);
    m_eventNotifier = new QSocketNotifier(eventFd, QSocketNotifier::Read);
    QObject::connect(m_eventNotifier, &QSocketNotifier::activated, m_eventNotifier, [this]() {
        processEvents();
    });
}

void GpioInput::processEvents()
{
    const int eventCount = gpiod_line_request_read_edge_events(
        m_request,
        m_eventBuffer,
        gpiod_edge_event_buffer_get_capacity(m_eventBuffer));

    for (int index = 0; index < eventCount; ++index) {
        gpiod_edge_event *event = gpiod_edge_event_buffer_get_event(m_eventBuffer, index);
        InputManager::setButton(
            InputManager::Button::A,
            gpiod_edge_event_get_event_type(event) == GPIOD_EDGE_EVENT_FALLING_EDGE);
    }
}

void GpioInput::stop()
{
    delete m_eventNotifier;
    m_eventNotifier = nullptr;

    if (m_eventBuffer) {
        gpiod_edge_event_buffer_free(m_eventBuffer);
        m_eventBuffer = nullptr;
    }

    if (m_request) {
        gpiod_line_request_release(m_request);
        m_request = nullptr;
    }

    if (m_chip) {
        gpiod_chip_close(m_chip);
        m_chip = nullptr;
    }
}
#else
GpioInput::GpioInput() = default;
GpioInput::~GpioInput() = default;
void GpioInput::start() {}
#endif