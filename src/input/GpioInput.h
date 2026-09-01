#pragma once

#ifdef PULSAR_HAS_GPIOD
#include <QSocketNotifier>

struct gpiod_chip;
struct gpiod_line_request;
struct gpiod_edge_event_buffer;
#endif

class GpioInput
{
public:
    GpioInput();
    ~GpioInput();

    GpioInput(const GpioInput &) = delete;
    GpioInput &operator=(const GpioInput &) = delete;

    void start();

private:
#ifdef PULSAR_HAS_GPIOD
    void processEvents();
    void stop();

    gpiod_chip *m_chip = nullptr;
    gpiod_line_request *m_request = nullptr;
    gpiod_edge_event_buffer *m_eventBuffer = nullptr;
    QSocketNotifier *m_eventNotifier = nullptr;
#endif
};