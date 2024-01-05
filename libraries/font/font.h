// Every symbol is represented as an 8 bytes of data - each byte representing a "row"
// Within a single byte row, each bit represents whether that column is on/off
const byte digits[][8] = {
  // 0
  {
    B00000000,
    B00011100,
    B00010100,
    B00010100,
    B00010100,
    B00010100,
    B00011100,
    B00000000
  },
  // 1
  {
    B00000000,
    B00010000,
    B00010000,
    B00010000,
    B00010000,
    B00010000,
    B00010000,
    B00000000
  },
  // 2
  {
    B00000000,
    B00011100,
    B00000100,
    B00000100,
    B00011100,
    B00010000,
    B00011100,
    B00000000
  },
   // 3
  {
    B00000000,
    B00011100,
    B00000100,
    B00000100,
    B00011100,
    B00000100,
    B00011100,
    B00000000
  },
  // 4
  {
    B00000000,
    B00010100,
    B00010100,
    B00010100,
    B00011100,
    B00000100,
    B00000100,
    B00000000
  },
  // 5
  {
    B00000000,
    B00011100,
    B00010000,
    B00010000,
    B00011100,
    B00000100,
    B00011100,
    B00000000
  },
  // 6
  {
    B00000000,
    B0011100,
    B0010000,
    B0010000,
    B0011100,
    B0010100,
    B0011100,
    B0000000
  },
  // 7
  {
    B00000000,
    B0011100,
    B0000100,
    B0000100,
    B0000100,
    B0000100,
    B0000100,
    B0000000
  },
  // 8
  {
    B00000000,
    B0011100,
    B0010100,
    B0010100,
    B0011100,
    B0010100,
    B0011100,
    B0000000
  },
  // 9
  {
    B00000000,
    B00011100,
    B00010100,
    B00010100,
    B00011100,
    B00000100,
    B00011100,
    B00000000,
  },
  // C
  {
    B00000000,
    B00111000,
    B01000100,
    B01000000,
    B01000000,
    B01000000,
    B01000100,
    B00111000,
  },
  // o
  {
    B00000000,
    B00000000,
    B00011100,
    B00100010,
    B00100010,
    B00100010,
    B00100010,
    B00011100,
   },
   //d
   {
    B00000000,
    B00000010,
    B00000010,
    B00011010,
    B00100110,
    B00100010,
    B00100010,
    B00011110,
    },
    //e
    {
    B00000000,
    B00000000,
    B00011100,
    B00100010,
    B00100010,
    B00111100,
    B00100000,
    B00011100,
    },
    // Space Invader
    {
    B00011000,
    B00111100,
    B01111110,
    B11011011,
    B11111111,
    B00100100,
    B01011010,
    B10100101,
    },
};

const byte uppercase[][8] = {
            //A
         {
            0b00001000,
            0b00010100,
            0b00010100,
            0b00011100,
            0b00010100,
            0b00010100,
            0b00000000
           },
            //B
         {0b11110,
            0b10001,
            0b10001,
            0b11110,
            0b10001,
            0b10001,
            0b11110},
            //C
           {0b01110,
            0b10001,
            0b10000,
            0b10000,
            0b10000,
            0b10001,
            0b01110},
            //D
           {0b11100,
            0b10010,
            0b10001,
            0b10001,
            0b10001,
            0b10010,
            0b11100},
            //E
           {0b11111,
            0b10000,
            0b10000,
            0b11110,
            0b10000,
            0b10000,
            0b11111},
            //F
           {0b11111,
            0b10000,
            0b10000,
            0b11110,
            0b10000,
            0b10000,
            0b10000},
            //G
           {0b01110,
            0b10001,
            0b10000,
            0b10111,
            0b10001,
            0b10001,
            0b01111},
            //H
           {0b10001,
            0b10001,
            0b10001,
            0b11111,
            0b10001,
            0b10001,
            0b10001},
            //I
           {0b01110,
            0b00100,
            0b00100,
            0b00100,
            0b00100,
            0b00100,
            0b01110},
            //J
           {0b00111,
            0b00010,
            0b00010,
            0b00010,
            0b00010,
            0b10010,
            0b01100},
            //K
           {0b10001,
            0b10010,
            0b10100,
            0b11000,
            0b10100,
            0b10010,
            0b10001},
            //L
           {0b10000,
            0b10000,
            0b10000,
            0b10000,
            0b10000,
            0b10000,
            0b11111},
            //M
           {0b10001,
            0b11011,
            0b10101,
            0b10101,
            0b10001,
            0b10001,
            0b10001},
            //N
           {0b10001,
            0b10001,
            0b11001,
            0b10101,
            0b10011,
            0b10001,
            0b10001},
            //O
           {0b01110,
            0b10001,
            0b10001,
            0b10001,
            0b10001,
            0b10001,
            0b01110},
            //P
           {0b11110,
            0b10001,
            0b10001,
            0b11110,
            0b10000,
            0b10000,
            0b10000},
            //Q
           {0b01110,
            0b10001,
            0b10001,
            0b10001,
            0b10101,
            0b10010,
            0b01101},
            //R
           {0b11110,
            0b10001,
            0b10001,
            0b11110,
            0b10100,
            0b10010,
            0b10001},
            //S
           {0b01111,
            0b10000,
            0b10000,
            0b01110,
            0b00001,
            0b00001,
            0b11110},
           //T
           {0b11111,
            0b10101,
            0b00100,
            0b00100,
            0b00100,
            0b00100,
            0b00100},
            //U
           {0b10001,
            0b10001,
            0b10001,
            0b10001,
            0b10001,
            0b10001,
            0b01110},
            //V
           {0b10001,
            0b10001,
            0b10001,
            0b10001,
            0b10001,
            0b01010,
            0b00100},
            //W
           {0b10001,
            0b10001,
            0b10001,
            0b10101,
            0b10101,
            0b10101,
            0b01010},
            //X
           {

            0b0010100,
            0b0010100,
            0b0010100,
            0b0001000,
            0b0010100,
            0b0010100,
            0b0000000
           },
            //Y
           {0b10001,
            0b10001,
            0b10001,
            0b01110,
            0b00100,
            0b00100,
            0b00100},
            //Z
           {0b11111,
            0b00001,
            0b00010,
            0b00100,
            0b01000,
            0b10000,
            0b11111},
            //[
           {0b01110,
            0b01000,
            0b01000,
            0b01000,
            0b01000,
            0b01000,
            0b01110},
            //
           {0b10001,
            0b01010,
            0b11111,
            0b00100,
            0b11111,
            0b00100,
            0b00100},
            //]
           {0b01110,
            0b00010,
            0b00010,
            0b00010,
            0b00010,
            0b00010,
            0b01110},
            //^
           {0b00100,
            0b01010,
            0b10001,
            0b00000,
            0b00000,
            0b00000,
            0b00000},
            //_
           {
            0b00000,
            0b00000,
            0b00000,
            0b00000,
            0b00000,
            0b00000,
            0b11111
      },
            //`
           {0b01000,
            0b00100,
            0b00010,
            0b00000,
            0b00000,
            0b00000,
            0b00000},
            //a
           {
            0b00000,
            0b00000,
            0b01110,
            0b00001,
            0b01111,
            0b10001,
            0b01111
      },
            //b
           {0b10000,
            0b10000,
            0b10110,
            0b11001,
            0b10001,
            0b10001,
            0b11110},
            //c
           {
            0b00000,
            0b00000,
            0b01110,
            0b10000,
            0b10000,
            0b10001,
            0b01110
      },
            //d
           {0b00001,
            0b00001,
            0b01101,
            0b10011,
            0b10001,
            0b10001,
            0b01111},
            //e
           {
            0b00000,
            0b00000,
            0b01110,
            0b10001,
            0b11111,
            0b10000,
            0b01110
      },
            //f
           {0b00110,
            0b01001,
            0b01000,
            0b11110,
            0b01000,
            0b01000,
            0b01000},
            //g
           {
            0b00000,
            0b01111,
            0b10001,
            0b10001,
            0b01111,
            0b00001,
            0b01110
      },
            //h
           {0b10000,
            0b10000,
            0b10110,
            0b11001,
            0b10001,
            0b10001,
            0b10001},
            //i
           {0b00100,
            0b00000,
            0b01100,
            0b00100,
            0b00100,
            0b00100,
            0b01110},
            //j
           {0b00010,
            0b00000,
            0b00110,
            0b00010,
            0b00010,
            0b10010,
            0b01100},
            //k
           {0b10000,
            0b10000,
            0b10010,
            0b10100,
            0b11000,
            0b10100,
            0b10010},
            //l
           {0b01100,
            0b00100,
            0b00100,
            0b00100,
            0b00100,
            0b00100,
            0b01110},
            //m
           {
            0b00000,
            0b00000,
            0b11010,
            0b10101,
            0b10101,
            0b10101,
            0b10101
      },
            //n
           {
            0b00000,
            0b00000,
            0b10110,
            0b11001,
            0b10001,
            0b10001,
            0b10001
      },
            //o
           {
            0b00000,
            0b00000,
            0b01110,
            0b10001,
            0b10001,
            0b10001,
            0b01110
      },
            //p
           {
            0b00000,
            0b00000,
            0b11110,
            0b10001,
            0b11110,
            0b10000,
            0b10000
      },
            //q
           {
            0b00000,
            0b00000,
            0b01101,
            0b10011,
            0b01111,
            0b00001,
            0b00001
      },
            //r
           {
            0b00000,
            0b00000,
            0b10110,
            0b11001,
            0b10000,
            0b10000,
            0b10000
      },
            //s
           {
            0b00000,
            0b00000,
            0b01110,
            0b10000,
            0b01110,
            0b00001,
            0b11110
      },
            //t
           {0b01000,
            0b01000,
            0b11111,
            0b01000,
            0b01000,
            0b01001,
            0b00110},
            //u
           {
            0b00000,
            0b00000,
            0b10001,
            0b10001,
            0b10001,
            0b10011,
            0b01101
      },
            //v
           {
            0b00000,
            0b00000,
            0b10001,
            0b10001,
            0b10001,
            0b01010,
            0b00100
      },
            //w
           {
            0b00000,
            0b00000,
            0b10001,
            0b10001,
            0b10101,
            0b10101,
            0b01010
      },
            //x
           {
            0b00000,
            0b00000,
            0b10001,
            0b01010,
            0b00100,
            0b01010,
            0b10001
      },
            //y
           {
            0b00000,
            0b00000,
            0b10001,
            0b10001,
            0b01111,
            0b00001,
            0b01110
      },
            //z
           {
            0b00000,
            0b00000,
            0b11111,
            0b00010,
            0b01000,
            0b10000,
            0b11111
      },
};
