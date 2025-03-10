#include <limits>
#include <csetjmp>
#include <cstdio>
#include <typeinfo>
#include <Windows.h>

#pragma clang diagnostic ignored "-Wdeprecated-declarations"

extern "C" {
	const void *CDECL memchr(const void *dest, int ch, size_t count) {
		const char *c = static_cast<const char *>(dest);

		while (count-- > 0) {
			if (*c == static_cast<unsigned char>(ch)) {
				return const_cast<char *>(c);
			}
		}

		return nullptr;
	}

	int CDECL memcmp(const void *lhs, const void *rhs, size_t count) {
		const unsigned char *l = static_cast<const unsigned char *>(lhs);
		const unsigned char *r = static_cast<const unsigned char *>(rhs);

		for (; count-- > 0; ++l, ++r) {
			if (*l < *r) {
				return -1;
			} else if (*l > *r) {
				return 1;
			}
		}

		return 0;
	}

	void *CDECL memset(void *dest, int ch, size_t count) {
		char *c = static_cast<char *>(dest);

		while (count-- > 0) {
			*c++ = static_cast<unsigned char>(ch);
		}

		return dest;
	}

	void *memcpy(void *dst, const void *src, size_t count) {
		char *d = static_cast<char *>(dst);
		const char *s = static_cast<const char *>(src);

		while (count-- > 0) {
			*d++ = *s++;
		}

		return dst;
	}

	void *memmove(void *dst, const void *src, size_t count) {
		char *d = static_cast<char *>(dst);
		const char *s = static_cast<const char *>(src);

		if (d < s) {
			return memcpy(dst, src, count);
		} else {
			for (d += count, s += count; count--; ) {
				*(--d) = *(--s);
			}
		}

		return dst;
	}

	char *strcpy(char *dst, const char *src) {
		const auto save = dst;

		while (*src)
			*dst++ = *src++;

		return save;
	}

	using AtExitFunction = void (*)();

	constexpr auto MAX_AT_EXIT = 32;
	AtExitFunction _at_exit[MAX_AT_EXIT];
	int _at_exit_size;

	int CDECL atexit(AtExitFunction f) {
		if (_at_exit_size >= MAX_AT_EXIT)
			return 1;

		_at_exit[_at_exit_size++] = f;

		return 0;
	}

	void CDECL abort() {
		ExitProcess(-1);
	}

	char *strerror(int) {
		return const_cast<char *>("An error occurred");
	}

	errno_t CDECL strerror_s(char *buf, rsize_t bufsz, errno_t errnum) {
		const auto s = strerror(errnum);
		const auto slen = strlen(s);

		if (slen < bufsz) {
			memcpy(buf, s, slen + 1);
			return 0;
		} else if (bufsz >= 4) {
			memcpy(buf, s, bufsz - 4);
			memcpy(buf + bufsz - 4, "...", 4);
			return ENOMEM;
		} else {
			memcpy(buf, s, bufsz - 1);
			buf[bufsz - 1] = '\0';
			return ENOMEM;
		}
	}

	int CDECL toupper(int c) {
		if (c >= 'a' && c <= 'z')
			return c ^ 32;

		return c;
	}

	int CDECL tolower(int c) {
		if (c >= 'A' && c <= 'Z')
			return c ^ 32;

		return c;
	}

	int CDECL isspace(int c) {
		return c == ' ' || c == '\r' || c == '\n' || c == '\t' || c == '\v' || c == '\f';
	}

	int CDECL isdigit(int c) {
		return c >= '0' && c <= '9';
	}

	void *CDECL malloc(size_t size) {
		return ::HeapAlloc(GetProcessHeap(), 0, size);
	}

	void *CDECL realloc(void *p, size_t num) {
		return p ? ::HeapReAlloc(GetProcessHeap(), 0, p, num) : malloc(num);
	}

	void *CDECL calloc(size_t num, size_t size) {
		size *= num;

		if (void *p = malloc(size)) {
			return memset(p, 0, size);
		}

		return nullptr;
	}

	void CDECL free(void *p) {
		::HeapFree(GetProcessHeap(), 0, p);
	}
}

template<typename C>
size_t cpp_strlen(const C *s) {
	size_t l = 0;

	while (*s++)
		++l;

	return l;
}

template<typename C>
C *cpp_strchr(const C *s, int ch) {
	while (*s != ch) {
		if (*s == 0)
			return nullptr;

		++s;
	}

	return const_cast<C *>(s);
}

template<typename I, typename C>
I cpp_strtoi(const C *s, C **end, int base) {
	while (isspace(*s)) {
		++s;
	}

	bool negative = *s == '-';
	if (*s == '+' || *s == '-')
		++s;

	if (base == 0) {
		if (*s == '0') {
			++s;

			if (*s == 'x' || *s == 'X') {
				base = 16;
				++s;
			} else {
				base = 8;
			}
		} else {
			base = 10;
		}
	}

	const C alpha[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
						'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
						'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
						'u', 'v', 'w', 'x', 'y', 'z', '\0' };
	I result{};
	bool range = false;

	while (const auto achr = cpp_strchr(alpha, tolower(*s))) {
		if (const auto digit = achr - alpha; digit < base) {
			if (std::is_signed_v<I> && negative) {
				if (result < (std::numeric_limits<I>::min)() / base) {
					range = true;
					break;
				}

				result *= base;

				if (result < (std::numeric_limits<I>::min)() + digit) {
					range = true;
					break;
				}

				result -= digit;
			} else {
				if (result > (std::numeric_limits<I>::max)() / base) {
					range = true;
					break;
				}

				result *= base;

				if (result > (std::numeric_limits<I>::max)() - digit) {
					range = true;
					break;
				}

				result += digit;
			}
		} else {
			break;
		}
	}

	if (end) {
		*end = const_cast<C *>(s);
	}

	if (range) {
		errno = ERANGE;
	}

	return !std::is_signed_v<I> && negative ? I(-I(result)) : result;
}

template<typename F, typename C>
F cpp_strtof(const C *s, C **end) {
	// TODO: implement conversions
	(void)s;
	(void)end;

	return {};
}

extern "C" {
	size_t CDECL strlen(const char *s) { return cpp_strlen(s); }
	size_t CDECL wcslen(const wchar_t *s) { return cpp_strlen(s); }

	char *CDECL strdup(const char *s) {
		const auto len = strlen(s) + 1;
		char *p = static_cast<char *>(malloc(len));

		if (p) {
			memcpy(p, s, len);
		}

		return p;
	}

	long CDECL strtol(const char *s, char **end, int base) { return cpp_strtoi<long>(s, end, base); }
	long long CDECL strtoll(const char *s, char **end, int base) { return cpp_strtoi<long long>(s, end, base); }
	unsigned long CDECL strtoul(const char *s, char **end, int base) { return cpp_strtoi<unsigned long>(s, end, base); }
	unsigned long long CDECL strtoull(const char *s, char **end, int base) { return cpp_strtoi<unsigned long long>(s, end, base); }
	float CDECL strtof(const char *s, char **end) { return cpp_strtof<float>(s, end); }
	double CDECL strtod(const char *s, char **end) { return cpp_strtof<double>(s, end); }
	long double CDECL strtold(const char *s, char **end) { return cpp_strtof<long double>(s, end); }

	long CDECL wcstol(const wchar_t *s, wchar_t **end, int base) { return cpp_strtoi<long>(s, end, base); }
	long long CDECL wcstoll(const wchar_t *s, wchar_t **end, int base) { return cpp_strtoi<long long>(s, end, base); }
	unsigned long CDECL wcstoul(const wchar_t *s, wchar_t **end, int base) { return cpp_strtoi<unsigned long>(s, end, base); }
	unsigned long long CDECL wcstoull(const wchar_t *s, wchar_t **end, int base) { return cpp_strtoi<unsigned long long>(s, end, base); }
	float CDECL wcstof(const wchar_t *s, wchar_t **end) { return cpp_strtof<float>(s, end); }
	double CDECL wcstod(const wchar_t *s, wchar_t **end) { return cpp_strtof<double>(s, end); }
	long double CDECL wcstold(const wchar_t *s, wchar_t **end) { return cpp_strtof<long double>(s, end); }

	void CDECL longjmp(jmp_buf env, int status) {
		(void)env;
		(void)status;
		// TODO
		abort();
	}
}

// Non standard
template<typename I, typename C>
char *cpp_itoa(I value, C *buffer, int radix, bool upper = false) {
	const auto save = buffer;
	const char *alpha = upper ? "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ" : "0123456789abcdefghijklmnopqrstuvwxyz";
	char result[std::numeric_limits<I>::digits10 + 1 + std::is_signed<I>::value]{};
	char *end = result + sizeof(result);
	char *ptr = end;

	if (value < 0) {
		do {
			const auto digit = -(value % radix);
			value /= radix;
			*(--ptr) = alpha[digit];
		} while (value);

		*(--ptr) = '-';
	} else {
		do {
			const auto digit = value % radix;
			value /= radix;
			*(--ptr) = alpha[digit];
		} while (value);
	}

	for (auto size = end - ptr; size--; ) {
		*buffer++ = *ptr++;
	}
	*buffer = '\0';

	return save;
}

template<typename Char, typename Out>
int cpp_sprintf(Out out, const Char *fmt, va_list arglist) {
	int nChars{};

	while (*fmt) {
		auto c = *fmt++;

		if (c == '%') {
			if (*fmt == '%') {
				out(c);
				++nChars;
				++fmt;
				continue;
			}

			bool leftJustified = false;
			enum class PositiveFill {
				None,
				Space,
				Plus
			} positiveFill = PositiveFill::None;
			bool alternate = false;
			char fillChar [[maybe_unused]] = ' ';

			switch (*fmt) {
				case '-': leftJustified = true; ++fmt; break;
				case '+': positiveFill = PositiveFill::Plus; ++fmt;  break;
				case ' ': if (positiveFill != PositiveFill::Plus) positiveFill = PositiveFill::Space; ++fmt; break;
				case '#': alternate = true; ++fmt; break;
				case '0': fillChar = '0'; ++fmt; break;
				default: break;
			}

			if (leftJustified)
				fillChar = ' ';

			enum class Length {
				hh,
				h,
				None,
				l,
				ll,
				j,
				z,
				t,
				L,
			} length = Length::None;

			switch (*fmt) {
				case 'h': 
					length = Length::h;
					if (*++fmt == 'h') {
						length = Length::hh;
						++fmt;
					}
					break;
				case 'l': 
					length = Length::l;
					if (*++fmt == 'l') {
						length = Length::ll;
						++fmt;
					}
					break;
				case 'j': length = Length::j; ++fmt; break;
				case 'z': length = Length::z; ++fmt; break;
				case 't': length = Length::t; ++fmt; break;
				case 'L': length = Length::L; ++fmt; break;
				default: break;
			}

			int radix = 0;
			bool uppercase = false;
			bool isSigned = false;

			switch (*fmt) {
				case 'd': [[fallthrough]];
				case 'i': radix = 10; isSigned = true; break;
				case 'u': radix = 10; break;
				case 'o': radix = 8; break;
				case 'x': radix = 16; break;
				case 'X': radix = 16; uppercase = true; break;
				default: return -nChars;
			}

			if (radix) {
				if (isSigned) {
					char result[std::numeric_limits<unsigned long long>::digits10 + 2]{};

					switch (length) {
						case Length::hh: cpp_itoa(static_cast<unsigned char>(va_arg(arglist, int)), result, radix, uppercase); break;
						case Length::h: cpp_itoa(static_cast<unsigned short>(va_arg(arglist, int)), result, radix, uppercase); break;
						case Length::None: cpp_itoa(va_arg(arglist, unsigned int), result, radix, uppercase); break;
						case Length::l: cpp_itoa(va_arg(arglist, unsigned long), result, radix, uppercase); break;
						case Length::ll: cpp_itoa(va_arg(arglist, unsigned long long), result, radix, uppercase); break;
						case Length::j: cpp_itoa(va_arg(arglist, uintmax_t), result, radix, uppercase); break;
						case Length::z: cpp_itoa(va_arg(arglist, size_t), result, radix, uppercase); break;
						case Length::t: cpp_itoa(va_arg(arglist, std::make_unsigned_t<ptrdiff_t>), result, radix, uppercase); break;
						default: break;
					}

					if (*result != '-') {
						switch (positiveFill) {
							case PositiveFill::Space: out(' '); ++nChars; break;
							case PositiveFill::Plus: out('+'); ++nChars; break;
							default: break;
						}
					}

					for (auto p = result; *p; ++p) {
						out(*p);
						++nChars;
					}
				} else {
					char result[std::numeric_limits<signed long long>::digits10 + 3]{};

					switch (length) {
						case Length::hh: cpp_itoa(static_cast<signed char>(va_arg(arglist, int)), result, radix, uppercase); break;
						case Length::h: cpp_itoa(static_cast<signed short>(va_arg(arglist, int)), result, radix, uppercase); break;
						case Length::None: cpp_itoa(va_arg(arglist, signed int), result, radix, uppercase); break;
						case Length::l: cpp_itoa(va_arg(arglist, signed long), result, radix, uppercase); break;
						case Length::ll: cpp_itoa(va_arg(arglist, signed long long), result, radix, uppercase); break;
						case Length::j: cpp_itoa(va_arg(arglist, intmax_t), result, radix, uppercase); break;
						case Length::z: cpp_itoa(va_arg(arglist, std::make_signed_t<size_t>), result, radix, uppercase); break;
						case Length::t: cpp_itoa(va_arg(arglist, ptrdiff_t), result, radix, uppercase); break;
						default: break;
					}

					switch (positiveFill) {
						case PositiveFill::Space: out(' '); ++nChars; break;
						case PositiveFill::Plus: out('+'); ++nChars; break;
						default: break;
					}

					if (alternate) {
						switch (radix) {
							case 16: out('0'); out(uppercase ? 'X' : 'x'); nChars += 2; break;
							case 8: out('0'); ++nChars; break;
							default: break;
						}
					}

					for (auto p = result; *p; ++p) {
						out(*p);
						++nChars;
					}
				}
			}

			++fmt;
		} else {
			out(c);
			++nChars;
		}
	}

	return nChars;
}

extern "C" {
	char *_itoa(int value, char *buffer, int radix) { return cpp_itoa(value, buffer, radix); }
	char *_ltoa(long value, char *buffer, int radix) { return cpp_itoa(value, buffer, radix); }
	char *_ultoa(unsigned long value, char *buffer, int radix) { return cpp_itoa(value, buffer, radix); }
	char *_i64toa(long long value, char *buffer, int radix) { return cpp_itoa(value, buffer, radix); }
	char *_u64toa(unsigned long long value, char *buffer, int radix) { return cpp_itoa(value, buffer, radix); }

	int _errno_val;

	int *CDECL _errno() {
		return &_errno_val;
	}

	int _sys_nerr_val;

	int *CDECL __sys_nerr() {
		return &_sys_nerr_val;
	}

	FILE *CDECL __acrt_iob_func(unsigned n) {
		switch (n) {
			case 0: return static_cast<FILE *>(GetStdHandle(STD_INPUT_HANDLE));
			case 1: return static_cast<FILE *>(GetStdHandle(STD_OUTPUT_HANDLE));
			case 2: return static_cast<FILE *>(GetStdHandle(STD_ERROR_HANDLE));
			default: return nullptr;
		}
	}

	void CDECL _fpreset() {
		/* Reset floating-point */
	}

	void CDECL _assert(const char *message, const char *filename, unsigned line) {
		(void)message;
		(void)filename;
		(void)line;
		abort();
	}

	void CDECL _wassert(const wchar_t *message, const wchar_t *filename, unsigned line) {
		(void)message;
		(void)filename;
		(void)line;
		abort();
	}

	void CDECL _purecall() {
		abort();
	}

	void CDECL __std_terminate() {
		abort();
	}

	int CDECL _setjmp(jmp_buf) {
		// TODO
		return 0;
	}

	int CDECL _setjmp3(jmp_buf *env, int count, ...) {
		(void)env;
		(void)count;
		// TODO
		return 0;
	}

	void __std_exception_copy(const __std_exception_data *from, __std_exception_data *to) {
		to->_DoFree = from->_DoFree;
		to->_What = from->_DoFree ? strdup(from->_What) : from->_What;
	}

	void __std_exception_destroy(__std_exception_data *p) {
		if (p->_DoFree) {
			free(const_cast<char *>(p->_What));
			p->_DoFree = false;
			p->_What = "";
		}
	}

	void WINAPI _CxxThrowException(void *pException, void *pThrowInfo) {
		(void)pException;
		(void)pThrowInfo;
		// TODO
	}

	void *__CxxFrameHandler3(void *pExcept,
							 void *pRN,
							 void *pContext,
							 void *pDC) {
		(void)pExcept;
		(void)pRN;
		(void)pContext;
		(void)pDC;
		// TODO
		return nullptr;
	}

	int _except_handler3(PEXCEPTION_RECORD exception_record,
						 PEXCEPTION_REGISTRATION_RECORD registration,
						 PCONTEXT context,
						 PEXCEPTION_REGISTRATION_RECORD dispatcher) {
		(void)exception_record;
		(void)registration;
		(void)context;
		(void)dispatcher;
		return 0;
		// return DISPOSITION_DISMISS;
		// return DISPOSITION_CONTINUE_SEARCH;
	}

	int CDECL __stdio_common_vswprintf(
		unsigned __int64 const options,
		wchar_t *const buffer,
		size_t const buffer_count,
		wchar_t const *const format,
		_locale_t const locale,
		va_list const arglist
	) {
		size_t size = 0;
		const auto Out = [buffer, buffer_count, &size](char c) {
			if (size < buffer_count) {
				buffer[size++] = c;
			}
		};

		(void)options;
		(void)locale;

		return cpp_sprintf(Out, format, arglist);
	}

	int CDECL __stdio_common_vfprintf(
		unsigned __int64 options,
		FILE *stream,
		char const *format,
		_locale_t locale,
		va_list argList
	) {
		const auto Out = [stream](char c) {
			(void)stream;
			(void)c;
			// fputc(c, stream);
		};

		(void)options;
		(void)locale;

		return cpp_sprintf(Out, format, argList);
	}

	int CDECL __stdio_common_vsprintf(
		unsigned __int64 options,
		char *buffer,
		size_t bufferCount,
		char const *format,
		_locale_t locale,
		va_list argList
	) {
		size_t size = 0;
		const auto Out = [buffer, bufferCount, &size](char c) {
			if (size < bufferCount) {
				buffer[size++] = c;
			}
		};

		(void)options;
		(void)locale;

		return cpp_sprintf(Out, format, argList);
	}

	uintptr_t CDECL _beginthreadex(void *security, unsigned stack_size, unsigned (WINAPI *start_address)(void *), void *arglist, unsigned initflag, unsigned *thrdaddr) {
		DWORD id{};

		if (const auto handle = ::CreateThread(static_cast<SECURITY_ATTRIBUTES *>(security),
											   stack_size,
											   reinterpret_cast<LPTHREAD_START_ROUTINE>(start_address),
											   arglist,
											   initflag,
											   &id)) {
			if (thrdaddr)
				*thrdaddr = id;

			return reinterpret_cast<uintptr_t>(handle);
		} else {
			errno = ::GetLastError();
			return 0;
		}
	}

	void CDECL _endthreadex(unsigned retval) {
		::ExitThread(retval);
	}

	__attribute((force_align_arg_pointer))
	int wWinMainCRTStartup() {
		_errno_val = 0;
		_sys_nerr_val = 0;

		return wWinMain(GetModuleHandle(NULL), NULL, ::GetCommandLineW(), SW_SHOW);
	}
}

// C++
void *operator new(size_t n) {
	return malloc(n);
}

void *operator new(size_t n, std::align_val_t) {
	return malloc(n);
}

void operator delete(void *p) noexcept {
	free(p);
}

void operator delete(void *p, size_t) noexcept {
	free(p);
}

void operator delete(void *p, size_t, std::align_val_t) noexcept {
	free(p);
}