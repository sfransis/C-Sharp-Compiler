int x;
int y;

func(int a, char* b, char* s)
{
   x = 42+12;
   y = 0;
   printf("x=%d\n",x);

   while (y < 2) {
	printf("Hello human!\n");
	y = y + 1;
   }

   if (x == 10) {
	printf("Equals 10!\n");
   }

   else {
      printf("This be the test\n");
   }

   if (x != 10) {
	printf("Does not equal 10!\n");
   }
}

main(int argc, char* argv)
{
   func(42, "goodbye","third arg");
}
