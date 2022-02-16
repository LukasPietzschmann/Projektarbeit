#pragma once

#include <seq.ch>

template <typename underlying_window_type>
class window_like {
public:
	explicit window_like(underlying_window_type* base) : m_underlying_window(base) {}
	virtual ~window_like() = default;

	virtual void add_n_str(const CH::str&, int x, int y) = 0;
	virtual void del_line(int x, int y) = 0;
	virtual void clear() = 0;

	virtual void prepare_refresh() const = 0;

	underlying_window_type* operator*() {
		return m_underlying_window;
	}

protected:
	underlying_window_type* m_underlying_window;
};