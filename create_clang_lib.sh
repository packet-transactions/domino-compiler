set -v
set -x

# Clean out all previous work
rm -rf obj_files all_objs
mkdir obj_files
mkdir all_objs

# Iterate through all .a files
for file in /usr/lib/llvm-3.5/lib/libclangAST.a \
            /usr/lib/llvm-3.5/lib/libclangAnalysis.a \
            /usr/lib/llvm-3.5/lib/libclangBasic.a \
            /usr/lib/llvm-3.5/lib/libclangDriver.a \
            /usr/lib/llvm-3.5/lib/libclangEdit.a \
            /usr/lib/llvm-3.5/lib/libclangFrontend.a \
            /usr/lib/llvm-3.5/lib/libclangFrontendTool.a \
            /usr/lib/llvm-3.5/lib/libclangLex.a\
            /usr/lib/llvm-3.5/lib/libclangParse.a \
            /usr/lib/llvm-3.5/lib/libclangSema.a\
            /usr/lib/llvm-3.5/lib/libclangASTMatchers.a\
            /usr/lib/llvm-3.5/lib/libclangRewrite.a\
            /usr/lib/llvm-3.5/lib/libclangRewriteFrontend.a\
            /usr/lib/llvm-3.5/lib/libclangSerialization.a\
            /usr/lib/llvm-3.5/lib/libclangTooling.a
do
  file_name=`echo $file | cut -d '/' -f 6`
  echo "file_name is $file_name"
  mkdir obj_files/$file_name
  cd obj_files/$file_name
  ar x $file
  cd ../..
done

for file in /usr/lib/llvm-3.5/lib/libclangAST.a \
            /usr/lib/llvm-3.5/lib/libclangAnalysis.a \
            /usr/lib/llvm-3.5/lib/libclangBasic.a \
            /usr/lib/llvm-3.5/lib/libclangDriver.a \
            /usr/lib/llvm-3.5/lib/libclangEdit.a \
            /usr/lib/llvm-3.5/lib/libclangFrontend.a \
            /usr/lib/llvm-3.5/lib/libclangFrontendTool.a \
            /usr/lib/llvm-3.5/lib/libclangLex.a\
            /usr/lib/llvm-3.5/lib/libclangParse.a \
            /usr/lib/llvm-3.5/lib/libclangSema.a\
            /usr/lib/llvm-3.5/lib/libclangASTMatchers.a\
            /usr/lib/llvm-3.5/lib/libclangRewrite.a\
            /usr/lib/llvm-3.5/lib/libclangRewriteFrontend.a\
            /usr/lib/llvm-3.5/lib/libclangSerialization.a\
            /usr/lib/llvm-3.5/lib/libclangTooling.a
do
  file_name=`echo $file | cut -d '/' -f 6`
  echo "file_name is $file_name"
  cd obj_files/$file_name
  for obj in *.o
  do
     mv $obj ../../all_objs/$file_name-$obj
  done
  cd ../..
done

cd all_objs
ar qc libclangall.a *.o
mv libclangall.a ..

cd ..
rm -rf all_objs
rm -rf obj_files
