#include "tests/test.h"

#include <stdio.h>
#include "base/bytebuffer.h"

void Test_ByteBuffer()
{
    vtw::ByteBuffer byteBuf;
    byteBuf << 7 << (long long int)1024 << 'x' << "Hello, ByteBuffer!";

    int i = 0;
    long long int l = 0;
    char c = '\0';
    const char *str = nullptr;

    byteBuf >> i >> l >> c >> str;

    printf("`%d %lld %c %s`\n", i, l, c, str);
}
