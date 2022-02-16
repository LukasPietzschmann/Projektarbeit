#pragma once

#include <seq.ch>

template <typename underlying_window_type>
class window_like {
public:
	explicit window_like(underlying_window_type* base, uint32_t width, uint32_t height) :
			m_underlying_window(base), m_screen_width(width), m_screen_height(height) {}
	virtual ~window_like() = default;

	virtual void add_n_str(const CH::str&, int x, int y) = 0;
	virtual void del_line(int x, int y) = 0;
	virtual void clear() = 0;

	virtual void prepare_refresh() const = 0;

	uint32_t get_height() const { return m_screen_height; }
	uint32_t get_width() const { return m_screen_width; }

	underlying_window_type* operator*() {
		return m_underlying_window;
	}

protected:
	underlying_window_type* m_underlying_window;
	uint32_t m_screen_width;
	uint32_t m_screen_height;
};