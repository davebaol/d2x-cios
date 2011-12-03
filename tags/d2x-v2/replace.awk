BEGIN {
    _ord_init();
    
    hex_major_ver = sprintf("0x%02x,0x%02x,0x%02x,0x%02x", and(rshift(major_ver,24),0xff), and(rshift(major_ver,16),0xff), and(rshift(major_ver,8),0xff), and(major_ver,0xff));

    hex_minor_ver = "";
    len = length(minor_ver);
    for (i=1; i<=16; i++) {
        c = (i > len) ? "\0" : substr(minor_ver, i, 1);
        hex_minor_ver = hex_minor_ver (i > 1 ? "," : "") sprintf("0x%02x",_ord_[c]);
    }
    
    vars["${MAJOR_VER}"]     = major_ver;
    vars["${MINOR_VER}"]     = minor_ver;
    vars["${HEX_MAJOR_VER}"] = hex_major_ver;
    vars["${HEX_MINOR_VER}"] = hex_minor_ver;
    vars["${IOS_REV}"]       = 21000 + major_ver;

}
 
function _ord_init(low, high, i, t)
     {
         low = sprintf("%c", 7) # BEL is ascii 7
         if (low == "\a") {    # regular ascii
             low = 0
             high = 127
         } else if (sprintf("%c", 128 + 7) == "\a") {
             # ascii, mark parity
             low = 128
             high = 255
         } else {        # ebcdic(!)
             low = 0
             high = 255
         }
     
         for (i = low; i <= high; i++) {
             t = sprintf("%c", i)
             _ord_[t] = i
         }
     }

{
    line_in = $0;
    line_out = "";
    for (;;) {
        match(line_in, /[$][{][^}.]+[}]/);

        if (RSTART == 0 && RLENGTH == -1) {
            line_out = line_out line_in;
            break;
        }

        var = substr(line_in, RSTART, RLENGTH);     
        value = (var in vars) ? vars[var] : var;
        line_out = line_out substr(line_in, 1, RSTART - 1) value;
        line_in = substr(line_in, RSTART + RLENGTH);
    }
    print line_out;
}