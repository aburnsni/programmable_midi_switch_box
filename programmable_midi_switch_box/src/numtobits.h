byte myfnNumToBits(int someNumber) {
  switch (someNumber) {
    case 0:
      return B00000001;
      break;
    case 1:
      return B00000010;
      break;
    case 2:
      return B00000100;
      break;
    case 3:
      return B00001000;
      break;
    case 4:
      return B00010000;
      break;
    case 5:
      return B00100000;
      break;
    case 6:
      return B01000000;
      break;
    case 7:
      return B10000000;
      break;
    default:
      return B00000000; // Error condition, displays three vertical bars
      break;   
  }
}