#pragma once
#include <dwmapi.h>
#include <string>
#include <unordered_map>

#include "findwindowiterator.hpp"
#include "windowclass.hpp"

class EventHook; // Forward declare to avoid circular deps

class Window {
private:
	static std::unordered_map<Window, std::wstring> m_ClassNames;
	static std::unordered_map<Window, std::wstring> m_Filenames;
	static std::unordered_map<Window, std::wstring> m_Titles;

	static const EventHook m_ChangeHook;
	static const EventHook m_DestroyHook;

	static void HandleChangeEvent(const DWORD, Window window, ...);
	static void HandleDestroyEvent(const DWORD, Window window, ...);

protected:
	HWND m_WindowHandle;

public:
	static const Window NullWindow;
	static const Window BroadcastWindow;
	static const Window MessageOnlyWindow;

	class FindEnum {
	private:
		std::wstring m_class;
		std::wstring m_name;
		HWND m_parent;
	public:
		inline FindEnum(std::wstring className = L"", std::wstring windowName = L"", Window parent = Window::NullWindow) :
			m_class(std::move(className)),
			m_name(std::move(windowName)),
			m_parent(parent)
		{ }

		inline FindWindowIterator begin()
		{
			return FindWindowIterator(&m_class, &m_name, m_parent);
		}

		inline const FindWindowIterator &end()
		{
			return FindWindowIterator::EndIterator;
		}
	};

	inline static Window Find(const std::wstring &className = L"", const std::wstring &windowName = L"", Window parent = Window::NullWindow, Window childAfter = Window::NullWindow)
	{
		return FindWindowEx(parent, childAfter, className.empty() ? NULL : className.c_str(), windowName.empty() ? NULL : windowName.c_str());
	}
	inline static Window Create(unsigned long dwExStyle, const std::wstring &className,
		const std::wstring &windowName, unsigned long dwStyle, int x = 0, int y = 0,
		int nWidth = 0, int nHeight = 0, Window parent = Window::NullWindow, HMENU hMenu = NULL,
		HINSTANCE hInstance = GetModuleHandle(NULL), void *lpParam = nullptr)
	{
		return CreateWindowEx(dwExStyle, className.c_str(), windowName.c_str(), dwStyle, x, y, nWidth, nHeight,
			parent, hMenu, hInstance, lpParam);
	}
	inline static Window Create(unsigned long dwExStyle, const WindowClass &winClass,
		const std::wstring &windowName, unsigned long dwStyle, int x = 0, int y = 0,
		int nWidth = 0, int nHeight = 0, Window parent = Window::NullWindow,
		HMENU hMenu = NULL, HINSTANCE hInstance = GetModuleHandle(NULL), void *lpParam = nullptr)
	{
		return CreateWindowEx(dwExStyle, winClass.atom(), windowName.c_str(), dwStyle, x, y, nWidth, nHeight,
			parent, hMenu, hInstance, lpParam);
	}
	inline static Window ForegroundWindow() noexcept
	{
		return GetForegroundWindow();
	}
	inline static Window DesktopWindow() noexcept
	{
		return GetDesktopWindow();
	}

	static void ClearCache();

	constexpr Window(HWND handle = Window::NullWindow) noexcept : m_WindowHandle(handle) { };
	const std::wstring &title() const;
	const std::wstring &classname() const;
	const std::wstring &filename() const;
	bool on_current_desktop() const;
	inline unsigned int state() const
	{
		const WINDOWPLACEMENT result = placement();
		return result.length != 0 ? result.showCmd : SW_SHOW;
	}
	inline bool show(int state = SW_SHOW) const
	{
		return ShowWindow(m_WindowHandle, state);
	}
	inline bool visible() const
	{
		return IsWindowVisible(m_WindowHandle);
	}
	inline bool valid() const
	{
		return IsWindow(m_WindowHandle);
	}
	inline explicit operator bool() const
	{
		return valid();
	}
	WINDOWPLACEMENT placement() const;
	inline HMONITOR monitor() const
	{
		return MonitorFromWindow(m_WindowHandle, MONITOR_DEFAULTTOPRIMARY);
	}
	inline LRESULT send_message(unsigned int message, WPARAM wparam = 0, LPARAM lparam = 0) const
	{
		return SendMessage(m_WindowHandle, message, wparam, lparam);
	}
	inline LRESULT send_message(const std::wstring &message, WPARAM wparam = 0, LPARAM lparam = 0) const
	{
		return send_message(RegisterWindowMessage(message.c_str()), wparam, lparam);
	}
	constexpr HWND handle() const noexcept
	{
		return m_WindowHandle;
	}
	constexpr operator HWND() const noexcept
	{
		return m_WindowHandle;
	}
	inline bool operator ==(Window right) const noexcept
	{
		return m_WindowHandle == right.m_WindowHandle;
	}
	inline bool operator !=(Window right) const noexcept
	{
		return !operator==(right);
	}

	template<typename T>
	T get_attribute(DWMWINDOWATTRIBUTE attrib) const;

	friend struct std::hash<Window>;
};

// Specialize std::hash to allow the use of Window as unordered_map key
namespace std {
	template<>
	struct hash<Window> {
		inline std::size_t operator()(Window k) const noexcept
		{
			static const std::hash<HWND> hasher;
			return hasher(k.m_WindowHandle);
		}
	};
}