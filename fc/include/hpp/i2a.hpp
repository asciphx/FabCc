#ifndef I2A_HPP
#define I2A_HPP
/*
 * This software is licensed under the AGPL-3.0 License.
 *
 * Copyright (C) 2023 Asciphx
 *
 * Permissions of this strongest copyleft license are conditioned on making available
 * complete source code of licensed works and modifications, which include larger works
 * using a licensed work, under the same license. Copyright and license notices must be
 * preserved. Contributors provide an express grant of patent rights. When a modified
 * version is used to provide a service over a network, the complete source code of
 * the modified version must be made available.
 */
#if defined(_WIN32)
#define __ALIGN(_) __declspec(align(_))
#define _INLINE __forceinline
#else
#define __ALIGN(_) __attribute__((aligned(_)))
#define _INLINE inline __attribute__((always_inline))
#endif
//#pragma warning(disable:4244)
static __ALIGN(1) const char _c3DigitsLut[0xbb9] =
"000001002003004005006007008009010011012013014015016017018019020021022023024025026027028029030031032033034035036037038039040041042043044045046047048049"
"050051052053054055056057058059060061062063064065066067068069070071072073074075076077078079080081082083084085086087088089090091092093094095096097098099"
"100101102103104105106107108109110111112113114115116117118119120121122123124125126127128129130131132133134135136137138139140141142143144145146147148149"
"150151152153154155156157158159160161162163164165166167168169170171172173174175176177178179180181182183184185186187188189190191192193194195196197198199"
"200201202203204205206207208209210211212213214215216217218219220221222223224225226227228229230231232233234235236237238239240241242243244245246247248249"
"250251252253254255256257258259260261262263264265266267268269270271272273274275276277278279280281282283284285286287288289290291292293294295296297298299"
"300301302303304305306307308309310311312313314315316317318319320321322323324325326327328329330331332333334335336337338339340341342343344345346347348349"
"350351352353354355356357358359360361362363364365366367368369370371372373374375376377378379380381382383384385386387388389390391392393394395396397398399"
"400401402403404405406407408409410411412413414415416417418419420421422423424425426427428429430431432433434435436437438439440441442443444445446447448449"
"450451452453454455456457458459460461462463464465466467468469470471472473474475476477478479480481482483484485486487488489490491492493494495496497498499"
"500501502503504505506507508509510511512513514515516517518519520521522523524525526527528529530531532533534535536537538539540541542543544545546547548549"
"550551552553554555556557558559560561562563564565566567568569570571572573574575576577578579580581582583584585586587588589590591592593594595596597598599"
"600601602603604605606607608609610611612613614615616617618619620621622623624625626627628629630631632633634635636637638639640641642643644645646647648649"
"650651652653654655656657658659660661662663664665666667668669670671672673674675676677678679680681682683684685686687688689690691692693694695696697698699"
"700701702703704705706707708709710711712713714715716717718719720721722723724725726727728729730731732733734735736737738739740741742743744745746747748749"
"750751752753754755756757758759760761762763764765766767768769770771772773774775776777778779780781782783784785786787788789790791792793794795796797798799"
"800801802803804805806807808809810811812813814815816817818819820821822823824825826827828829830831832833834835836837838839840841842843844845846847848849"
"850851852853854855856857858859860861862863864865866867868869870871872873874875876877878879880881882883884885886887888889890891892893894895896897898899"
"900901902903904905906907908909910911912913914915916917918919920921922923924925926927928929930931932933934935936937938939940941942943944945946947948949"
"950951952953954955956957958959960961962963964965966967968969970971972973974975976977978979980981982983984985986987988989990991992993994995996997998999";
static __ALIGN(1) const char _c2DigitsLut[0xc9] =
"0001020304050607080910111213141516171819202122232425262728293031323334353637383940414243444546474849"
"5051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979899";
//The fastest utoa fuction
_INLINE static unsigned char u2a(char* c, unsigned int i) {
  if (i < 10000u) {
    unsigned int a = (i / 100u) << 1; unsigned char l = 1;
    if (i > 999u) *c++ = _c2DigitsLut[a], ++l;
    if (i > 99u) *c++ = _c2DigitsLut[a + 1], ++l;
    a = (i % 100u) << 1;
    if (i > 9u) *c++ = _c2DigitsLut[a], ++l;
    *c++ = _c2DigitsLut[a + 1];
    *c = 0; return l;
  } else if (i < 100000000u) {
    const unsigned int a = (i / 1000000u) << 1;
    const unsigned int b = i % 1000000u; unsigned char l = 5;
    const unsigned int d = (b / 1000u) * 3;
    const unsigned int e = (b % 1000u) * 3;
    if (i > 9999999u) *c++ = _c2DigitsLut[a], ++l;
    if (i > 999999u) *c++ = _c2DigitsLut[a + 1], ++l;
    if (i > 99999u) *c++ = _c3DigitsLut[d], ++l;
    *c++ = _c3DigitsLut[d + 1];
    *c++ = _c3DigitsLut[d + 2];
    *c++ = _c3DigitsLut[e];
    *c++ = _c3DigitsLut[e + 1];
    *c++ = _c3DigitsLut[e + 2];
    *c = 0; return l;
  } else if (i < 1000000000u) {
    unsigned int V = i / 1000u;
    unsigned int F = 3 * (i - (V * 1000u));
    c[6] = _c3DigitsLut[F];
    c[7] = _c3DigitsLut[++F];
    c[8] = _c3DigitsLut[++F];
    i = V / 1000u;
    F = 3 * (V - (i * 1000u));
    c[3] = _c3DigitsLut[F];
    c[4] = _c3DigitsLut[++F];
    c[5] = _c3DigitsLut[++F];
    V = i / 1000u;
    F = 3 * (i - (V * 1000u));
    c[0] = _c3DigitsLut[F];
    c[1] = _c3DigitsLut[++F];
    c[2] = _c3DigitsLut[++F];
    c[9] = 0; return 9;
  } else {
    unsigned int V = i / 1000u;
    unsigned int F = 3 * (i - (V * 1000u));
    c[7] = _c3DigitsLut[F];
    c[8] = _c3DigitsLut[++F];
    c[9] = _c3DigitsLut[++F];
    i = V / 1000u;
    F = 3 * (V - (i * 1000u));
    c[4] = _c3DigitsLut[F];
    c[5] = _c3DigitsLut[++F];
    c[6] = _c3DigitsLut[++F];
    V = i / 1000u;
    F = 3 * (i - (V * 1000u));
    c[0] = V + 0x30;
    c[1] = _c3DigitsLut[F];
    c[2] = _c3DigitsLut[++F];
    c[3] = _c3DigitsLut[++F];
    c[10] = 0; return 10;
  }
}
//The fastest itoa fuction
_INLINE static unsigned char i2a(char* c, int i) {
  if (i < 0) { *c = 45; return u2a(++c, ~--i); } return u2a(c, i);
}
//The fastest u64toa fuction
_INLINE static unsigned char u64toa(char* c, unsigned long long i) {
  if (i < 0x100000000ULL) {
    return u2a(c, static_cast<unsigned int>(i));
  } else if (i < 10000000000ULL) {
    unsigned int F = static_cast<unsigned int>(i / 10000000u) * 3;
    unsigned char k = 0; i %= 10000000u;
    c[k] = _c3DigitsLut[F];
    c[++k] = _c3DigitsLut[++F];
    c[++k] = _c3DigitsLut[++F];
    F = static_cast<unsigned int>(i / 10000u) * 3; i %= 10000u;
    c[++k] = _c3DigitsLut[F];
    c[++k] = _c3DigitsLut[++F];
    c[++k] = _c3DigitsLut[++F];
    F = static_cast<unsigned int>(i / 10u * 3); i %= 10u;
    c[++k] = _c3DigitsLut[F];
    c[++k] = _c3DigitsLut[++F];
    c[++k] = _c3DigitsLut[++F];
    c[++k] = static_cast<char>(i + 0x30);
    c[++k] = 0; return k;
  } else {
    unsigned char k = u2a(c, static_cast<unsigned int>(i / 10000000000ULL));
    i %= 10000000000ULL; unsigned int F = static_cast<unsigned int>(i / 1000000000ULL);
    F = static_cast<unsigned int>(i / 10000000u) * 3; i %= 10000000u;
    c[k] = _c3DigitsLut[F];
    c[++k] = _c3DigitsLut[++F];
    c[++k] = _c3DigitsLut[++F];
    F = static_cast<unsigned int>(i / 10000u) * 3; i %= 10000u;
    c[++k] = _c3DigitsLut[F];
    c[++k] = _c3DigitsLut[++F];
    c[++k] = _c3DigitsLut[++F];
    F = static_cast<unsigned int>(i / 10u) * 3; i %= 10u;
    c[++k] = _c3DigitsLut[F];
    c[++k] = _c3DigitsLut[++F];
    c[++k] = _c3DigitsLut[++F];
    c[++k] = static_cast<char>(i + 0x30);
    c[++k] = 0; return k;
  }
}
//The fastest i64toa fuction
_INLINE static unsigned char i64toa(char* c, long long i) {
  if (i < 0) { *c = 45; return u64toa(++c, ~--i); } return u64toa(c, i);
}
#undef __ALIGN
#undef _INLINE
#endif // I2A_HPP
