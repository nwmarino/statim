; ModuleID = 'rt.c'
source_filename = "rt.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@.str = private unnamed_addr constant [5 x i8] c"true\00", align 1
@.str.1 = private unnamed_addr constant [6 x i8] c"false\00", align 1
@.str.2 = private unnamed_addr constant [2 x i8] c"0\00", align 1
@.str.3 = private unnamed_addr constant [2 x i8] c".\00", align 1
@.str.4 = private unnamed_addr constant [3 x i8] c"0x\00", align 1

; Function Attrs: noinline noreturn nounwind optnone
define dso_local void @exit(i64 noundef %0) #0 {
  %2 = alloca i64, align 8
  store i64 %0, ptr %2, align 8
  %3 = load i64, ptr %2, align 8
  call void asm sideeffect "movq $0, %rdi\0Amovq $$60, %rax\0Asyscall\0A", "r,~{rax},~{rdi},~{dirflag},~{fpsr},~{flags}"(i64 %3) #4, !srcloc !5
  unreachable
}

; Function Attrs: naked noinline nounwind optnone
define dso_local void @_start() #1 {
  call void asm sideeffect "xorl %ebp, %ebp\0Amovq (%rsp), %rdi\0Aleaq 8(%rsp), %rsi\0Acall main\0Amovq %rax, %rdi\0Amovq $$60, %rax\0Asyscall\0A", "~{memory},~{dirflag},~{fpsr},~{flags}"() #4, !srcloc !6
  unreachable
}

; Function Attrs: naked noinline noreturn nounwind optnone
define dso_local void @__abort() #2 {
  call void asm sideeffect "movq $$62, %rax\0Amovq $$0, %rdi\0Amovq $$6, %rsi\0Asyscall\0A", "~{rax},~{rdi},~{rsi},~{dirflag},~{fpsr},~{flags}"() #4, !srcloc !7
  unreachable
}

; Function Attrs: noinline noreturn nounwind optnone
define dso_local void @__panic(ptr noundef %0, i64 noundef %1) #0 {
  %3 = alloca ptr, align 8
  %4 = alloca i64, align 8
  store ptr %0, ptr %3, align 8
  store i64 %1, ptr %4, align 8
  %5 = load ptr, ptr %3, align 8
  %6 = load i64, ptr %4, align 8
  call void asm sideeffect "movq $$1, %rax\0Amovq $$2, %rdi\0Amovq $0, %rsi\0Amovq $1, %rdx\0Asyscall\0Acall __abort\0A", "r,r,~{rax},~{rdi},~{rsi},~{rdi},~{dirflag},~{fpsr},~{flags}"(ptr %5, i64 %6) #4, !srcloc !8
  unreachable
}

; Function Attrs: noinline nounwind optnone
define dso_local void @__memcpy(ptr noundef %0, ptr noundef %1, i64 noundef %2) #3 {
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca i64, align 8
  %7 = alloca ptr, align 8
  %8 = alloca ptr, align 8
  store ptr %0, ptr %4, align 8
  store ptr %1, ptr %5, align 8
  store i64 %2, ptr %6, align 8
  %9 = load ptr, ptr %4, align 8
  store ptr %9, ptr %7, align 8
  %10 = load ptr, ptr %5, align 8
  store ptr %10, ptr %8, align 8
  br label %11

11:                                               ; preds = %14, %3
  %12 = load i64, ptr %6, align 8
  %13 = icmp ugt i64 %12, 0
  br i1 %13, label %14, label %24

14:                                               ; preds = %11
  %15 = load ptr, ptr %8, align 8
  %16 = load i64, ptr %6, align 8
  %17 = getelementptr inbounds nuw i8, ptr %15, i64 %16
  %18 = load i8, ptr %17, align 1
  %19 = load ptr, ptr %7, align 8
  %20 = load i64, ptr %6, align 8
  %21 = getelementptr inbounds nuw i8, ptr %19, i64 %20
  store i8 %18, ptr %21, align 1
  %22 = load i64, ptr %6, align 8
  %23 = add i64 %22, -1
  store i64 %23, ptr %6, align 8
  br label %11, !llvm.loop !9

24:                                               ; preds = %11
  ret void
}

; Function Attrs: noinline nounwind optnone
define dso_local void @__memset(ptr noundef %0, i8 noundef signext %1, i64 noundef %2) #3 {
  %4 = alloca ptr, align 8
  %5 = alloca i8, align 1
  %6 = alloca i64, align 8
  %7 = alloca ptr, align 8
  store ptr %0, ptr %4, align 8
  store i8 %1, ptr %5, align 1
  store i64 %2, ptr %6, align 8
  %8 = load ptr, ptr %4, align 8
  store ptr %8, ptr %7, align 8
  br label %9

9:                                                ; preds = %12, %3
  %10 = load i64, ptr %6, align 8
  %11 = icmp ugt i64 %10, 0
  br i1 %11, label %12, label %19

12:                                               ; preds = %9
  %13 = load i8, ptr %5, align 1
  %14 = load ptr, ptr %7, align 8
  %15 = load i64, ptr %6, align 8
  %16 = getelementptr inbounds nuw i8, ptr %14, i64 %15
  store i8 %13, ptr %16, align 1
  %17 = load i64, ptr %6, align 8
  %18 = add i64 %17, -1
  store i64 %18, ptr %6, align 8
  br label %9, !llvm.loop !11

19:                                               ; preds = %9
  ret void
}

; Function Attrs: noinline nounwind optnone
define dso_local i64 @__strlen(ptr noundef %0) #3 {
  %2 = alloca ptr, align 8
  %3 = alloca i64, align 8
  store ptr %0, ptr %2, align 8
  store i64 0, ptr %3, align 8
  br label %4

4:                                                ; preds = %12, %1
  %5 = load ptr, ptr %2, align 8
  %6 = load i64, ptr %3, align 8
  %7 = add i64 %6, 1
  store i64 %7, ptr %3, align 8
  %8 = getelementptr inbounds nuw i8, ptr %5, i64 %6
  %9 = load i8, ptr %8, align 1
  %10 = sext i8 %9 to i32
  %11 = icmp ne i32 %10, 0
  br i1 %11, label %12, label %13

12:                                               ; preds = %4
  br label %4, !llvm.loop !12

13:                                               ; preds = %4
  %14 = load i64, ptr %3, align 8
  ret i64 %14
}

; Function Attrs: noinline nounwind optnone
define dso_local void @__print(ptr noundef %0) #3 {
  %2 = alloca ptr, align 8
  %3 = alloca i64, align 8
  store ptr %0, ptr %2, align 8
  %4 = load ptr, ptr %2, align 8
  %5 = call i64 @__strlen(ptr noundef %4) #5
  store i64 %5, ptr %3, align 8
  %6 = load ptr, ptr %2, align 8
  %7 = load i64, ptr %3, align 8
  call void asm sideeffect "movq $$1, %rax\0Amovq $$1, %rdi\0Amovq $0, %rsi\0Amovq $1, %rdx\0Asyscall\0A", "r,r,~{rax},~{rdi},~{rsi},~{rdi},~{dirflag},~{fpsr},~{flags}"(ptr %6, i64 %7) #4, !srcloc !13
  ret void
}

; Function Attrs: noinline nounwind optnone
define dso_local void @__print_fixed(ptr noundef %0, i64 noundef %1) #3 {
  %3 = alloca ptr, align 8
  %4 = alloca i64, align 8
  store ptr %0, ptr %3, align 8
  store i64 %1, ptr %4, align 8
  %5 = load ptr, ptr %3, align 8
  %6 = load i64, ptr %4, align 8
  call void asm sideeffect "movq $$1, %rax\0Amovq $$1, %rdi\0Amovq $0, %rsi\0Amovq $1, %rdx\0Asyscall\0A", "r,r,~{rax},~{rdi},~{rsi},~{rdi},~{dirflag},~{fpsr},~{flags}"(ptr %5, i64 %6) #4, !srcloc !14
  ret void
}

; Function Attrs: noinline nounwind optnone
define dso_local void @__err(ptr noundef %0) #3 {
  %2 = alloca ptr, align 8
  %3 = alloca i64, align 8
  store ptr %0, ptr %2, align 8
  %4 = load ptr, ptr %2, align 8
  %5 = call i64 @__strlen(ptr noundef %4) #5
  store i64 %5, ptr %3, align 8
  %6 = load ptr, ptr %2, align 8
  %7 = load i64, ptr %3, align 8
  call void asm sideeffect "movq $$1, %rax\0Amovq $$2, %rdi\0Amovq $0, %rsi\0Amovq $1, %rdx\0Asyscall\0A", "r,r,~{rax},~{rdi},~{rsi},~{rdi},~{dirflag},~{fpsr},~{flags}"(ptr %6, i64 %7) #4, !srcloc !15
  ret void
}

; Function Attrs: noinline nounwind optnone
define dso_local void @__err_fixed(ptr noundef %0, i64 noundef %1) #3 {
  %3 = alloca ptr, align 8
  %4 = alloca i64, align 8
  store ptr %0, ptr %3, align 8
  store i64 %1, ptr %4, align 8
  %5 = load ptr, ptr %3, align 8
  %6 = load i64, ptr %4, align 8
  call void asm sideeffect "movq $$1, %rax\0Amovq $$2, %rdi\0Amovq $0, %rsi\0Amovq $1, %rdx\0Asyscall\0A", "r,r,~{rax},~{rdi},~{rsi},~{rdi},~{dirflag},~{fpsr},~{flags}"(ptr %5, i64 %6) #4, !srcloc !16
  ret void
}

; Function Attrs: noinline nounwind optnone
define dso_local void @__print_fd(i64 noundef %0, ptr noundef %1) #3 {
  %3 = alloca i64, align 8
  %4 = alloca ptr, align 8
  %5 = alloca i64, align 8
  store i64 %0, ptr %3, align 8
  store ptr %1, ptr %4, align 8
  %6 = load ptr, ptr %4, align 8
  %7 = call i64 @__strlen(ptr noundef %6) #5
  store i64 %7, ptr %5, align 8
  %8 = load i64, ptr %3, align 8
  %9 = load ptr, ptr %4, align 8
  %10 = load i64, ptr %5, align 8
  call void asm sideeffect "movq $$1, %rax\0Amovq $0, %rdi\0Amovq $1, %rsi\0Amovq $2, %rdx\0Asyscall\0A", "r,r,r,~{rax},~{rdi},~{rsi},~{rdi},~{dirflag},~{fpsr},~{flags}"(i64 %8, ptr %9, i64 %10) #4, !srcloc !17
  ret void
}

; Function Attrs: noinline nounwind optnone
define dso_local void @__print_fd_fixed(i64 noundef %0, ptr noundef %1, i64 noundef %2) #3 {
  %4 = alloca i64, align 8
  %5 = alloca ptr, align 8
  %6 = alloca i64, align 8
  store i64 %0, ptr %4, align 8
  store ptr %1, ptr %5, align 8
  store i64 %2, ptr %6, align 8
  %7 = load i64, ptr %4, align 8
  %8 = load ptr, ptr %5, align 8
  %9 = load i64, ptr %6, align 8
  call void asm sideeffect "movq $$1, %rax\0Amovq $0, %rdi\0Amovq $1, %rsi\0Amovq $2, %rdx\0Asyscall\0A", "r,r,r,~{rax},~{rdi},~{rsi},~{rdi},~{dirflag},~{fpsr},~{flags}"(i64 %7, ptr %8, i64 %9) #4, !srcloc !18
  ret void
}

; Function Attrs: noinline nounwind optnone
define dso_local void @__print_bool(i64 noundef %0, i8 noundef signext %1) #3 {
  %3 = alloca i64, align 8
  %4 = alloca i8, align 1
  store i64 %0, ptr %3, align 8
  store i8 %1, ptr %4, align 1
  %5 = load i8, ptr %4, align 1
  %6 = icmp ne i8 %5, 0
  br i1 %6, label %7, label %9

7:                                                ; preds = %2
  %8 = load i64, ptr %3, align 8
  call void @__print_fd_fixed(i64 noundef %8, ptr noundef @.str, i64 noundef 4) #5
  br label %11

9:                                                ; preds = %2
  %10 = load i64, ptr %3, align 8
  call void @__print_fd_fixed(i64 noundef %10, ptr noundef @.str.1, i64 noundef 5) #5
  br label %11

11:                                               ; preds = %9, %7
  ret void
}

; Function Attrs: noinline nounwind optnone
define dso_local void @__print_char(i64 noundef %0, i8 noundef signext %1) #3 {
  %3 = alloca i64, align 8
  %4 = alloca i8, align 1
  store i64 %0, ptr %3, align 8
  store i8 %1, ptr %4, align 1
  %5 = load i64, ptr %3, align 8
  call void @__print_fd_fixed(i64 noundef %5, ptr noundef %4, i64 noundef 1) #5
  ret void
}

; Function Attrs: noinline nounwind optnone
define dso_local void @__print_si(i64 noundef %0, i64 noundef %1, i64 noundef %2) #3 {
  %4 = alloca i64, align 8
  %5 = alloca i64, align 8
  %6 = alloca i64, align 8
  %7 = alloca [65 x i8], align 16
  %8 = alloca i64, align 8
  %9 = alloca i8, align 1
  %10 = alloca i64, align 8
  %11 = alloca i64, align 8
  %12 = alloca i64, align 8
  %13 = alloca i8, align 1
  store i64 %0, ptr %4, align 8
  store i64 %1, ptr %5, align 8
  store i64 %2, ptr %6, align 8
  store i64 0, ptr %8, align 8
  store i8 0, ptr %9, align 1
  %14 = load i64, ptr %5, align 8
  %15 = icmp eq i64 %14, 0
  br i1 %15, label %16, label %18

16:                                               ; preds = %3
  %17 = load i64, ptr %4, align 8
  call void @__print_fd(i64 noundef %17, ptr noundef @.str.2) #5
  br label %89

18:                                               ; preds = %3
  %19 = load i64, ptr %5, align 8
  %20 = icmp slt i64 %19, 0
  br i1 %20, label %21, label %24

21:                                               ; preds = %18
  store i8 1, ptr %9, align 1
  %22 = load i64, ptr %5, align 8
  %23 = sub nsw i64 0, %22
  store i64 %23, ptr %5, align 8
  br label %24

24:                                               ; preds = %21, %18
  br label %25

25:                                               ; preds = %49, %24
  %26 = load i64, ptr %5, align 8
  %27 = icmp ne i64 %26, 0
  br i1 %27, label %28, label %53

28:                                               ; preds = %25
  %29 = load i64, ptr %5, align 8
  %30 = load i64, ptr %6, align 8
  %31 = srem i64 %29, %30
  store i64 %31, ptr %10, align 8
  %32 = load i64, ptr %10, align 8
  %33 = icmp sgt i64 %32, 9
  br i1 %33, label %34, label %42

34:                                               ; preds = %28
  %35 = load i64, ptr %10, align 8
  %36 = sub nsw i64 %35, 10
  %37 = add nsw i64 %36, 97
  %38 = trunc i64 %37 to i8
  %39 = load i64, ptr %8, align 8
  %40 = add nsw i64 %39, 1
  store i64 %40, ptr %8, align 8
  %41 = getelementptr inbounds [65 x i8], ptr %7, i64 0, i64 %39
  store i8 %38, ptr %41, align 1
  br label %49

42:                                               ; preds = %28
  %43 = load i64, ptr %10, align 8
  %44 = add nsw i64 %43, 48
  %45 = trunc i64 %44 to i8
  %46 = load i64, ptr %8, align 8
  %47 = add nsw i64 %46, 1
  store i64 %47, ptr %8, align 8
  %48 = getelementptr inbounds [65 x i8], ptr %7, i64 0, i64 %46
  store i8 %45, ptr %48, align 1
  br label %49

49:                                               ; preds = %42, %34
  %50 = load i64, ptr %5, align 8
  %51 = load i64, ptr %6, align 8
  %52 = sdiv i64 %50, %51
  store i64 %52, ptr %5, align 8
  br label %25, !llvm.loop !19

53:                                               ; preds = %25
  %54 = load i8, ptr %9, align 1
  %55 = sext i8 %54 to i32
  %56 = icmp eq i32 %55, 1
  br i1 %56, label %57, label %61

57:                                               ; preds = %53
  %58 = load i64, ptr %8, align 8
  %59 = add nsw i64 %58, 1
  store i64 %59, ptr %8, align 8
  %60 = getelementptr inbounds [65 x i8], ptr %7, i64 0, i64 %58
  store i8 45, ptr %60, align 1
  br label %61

61:                                               ; preds = %57, %53
  %62 = load i64, ptr %8, align 8
  %63 = getelementptr inbounds [65 x i8], ptr %7, i64 0, i64 %62
  store i8 0, ptr %63, align 1
  store i64 0, ptr %11, align 8
  %64 = load i64, ptr %8, align 8
  %65 = sub nsw i64 %64, 1
  store i64 %65, ptr %12, align 8
  br label %66

66:                                               ; preds = %70, %61
  %67 = load i64, ptr %11, align 8
  %68 = load i64, ptr %12, align 8
  %69 = icmp slt i64 %67, %68
  br i1 %69, label %70, label %86

70:                                               ; preds = %66
  %71 = load i64, ptr %11, align 8
  %72 = getelementptr inbounds [65 x i8], ptr %7, i64 0, i64 %71
  %73 = load i8, ptr %72, align 1
  store i8 %73, ptr %13, align 1
  %74 = load i64, ptr %12, align 8
  %75 = getelementptr inbounds [65 x i8], ptr %7, i64 0, i64 %74
  %76 = load i8, ptr %75, align 1
  %77 = load i64, ptr %11, align 8
  %78 = getelementptr inbounds [65 x i8], ptr %7, i64 0, i64 %77
  store i8 %76, ptr %78, align 1
  %79 = load i8, ptr %13, align 1
  %80 = load i64, ptr %12, align 8
  %81 = getelementptr inbounds [65 x i8], ptr %7, i64 0, i64 %80
  store i8 %79, ptr %81, align 1
  %82 = load i64, ptr %12, align 8
  %83 = add nsw i64 %82, -1
  store i64 %83, ptr %12, align 8
  %84 = load i64, ptr %11, align 8
  %85 = add nsw i64 %84, 1
  store i64 %85, ptr %11, align 8
  br label %66, !llvm.loop !20

86:                                               ; preds = %66
  %87 = load i64, ptr %4, align 8
  %88 = getelementptr inbounds [65 x i8], ptr %7, i64 0, i64 0
  call void @__print_fd(i64 noundef %87, ptr noundef %88) #5
  br label %89

89:                                               ; preds = %86, %16
  ret void
}

; Function Attrs: noinline nounwind optnone
define dso_local void @__print_ui(i64 noundef %0, i64 noundef %1, i64 noundef %2) #3 {
  %4 = alloca i64, align 8
  %5 = alloca i64, align 8
  %6 = alloca i64, align 8
  %7 = alloca [65 x i8], align 16
  %8 = alloca i64, align 8
  %9 = alloca i64, align 8
  %10 = alloca i64, align 8
  %11 = alloca i64, align 8
  %12 = alloca i8, align 1
  store i64 %0, ptr %4, align 8
  store i64 %1, ptr %5, align 8
  store i64 %2, ptr %6, align 8
  store i64 0, ptr %8, align 8
  %13 = load i64, ptr %5, align 8
  %14 = icmp eq i64 %13, 0
  br i1 %14, label %15, label %17

15:                                               ; preds = %3
  %16 = load i64, ptr %4, align 8
  call void @__print_fd(i64 noundef %16, ptr noundef @.str.2) #5
  br label %74

17:                                               ; preds = %3
  br label %18

18:                                               ; preds = %42, %17
  %19 = load i64, ptr %5, align 8
  %20 = icmp ne i64 %19, 0
  br i1 %20, label %21, label %46

21:                                               ; preds = %18
  %22 = load i64, ptr %5, align 8
  %23 = load i64, ptr %6, align 8
  %24 = urem i64 %22, %23
  store i64 %24, ptr %9, align 8
  %25 = load i64, ptr %9, align 8
  %26 = icmp ugt i64 %25, 9
  br i1 %26, label %27, label %35

27:                                               ; preds = %21
  %28 = load i64, ptr %9, align 8
  %29 = sub i64 %28, 10
  %30 = add i64 %29, 97
  %31 = trunc i64 %30 to i8
  %32 = load i64, ptr %8, align 8
  %33 = add nsw i64 %32, 1
  store i64 %33, ptr %8, align 8
  %34 = getelementptr inbounds [65 x i8], ptr %7, i64 0, i64 %32
  store i8 %31, ptr %34, align 1
  br label %42

35:                                               ; preds = %21
  %36 = load i64, ptr %9, align 8
  %37 = add i64 %36, 48
  %38 = trunc i64 %37 to i8
  %39 = load i64, ptr %8, align 8
  %40 = add nsw i64 %39, 1
  store i64 %40, ptr %8, align 8
  %41 = getelementptr inbounds [65 x i8], ptr %7, i64 0, i64 %39
  store i8 %38, ptr %41, align 1
  br label %42

42:                                               ; preds = %35, %27
  %43 = load i64, ptr %5, align 8
  %44 = load i64, ptr %6, align 8
  %45 = udiv i64 %43, %44
  store i64 %45, ptr %5, align 8
  br label %18, !llvm.loop !21

46:                                               ; preds = %18
  %47 = load i64, ptr %8, align 8
  %48 = getelementptr inbounds [65 x i8], ptr %7, i64 0, i64 %47
  store i8 0, ptr %48, align 1
  store i64 0, ptr %10, align 8
  %49 = load i64, ptr %8, align 8
  %50 = sub nsw i64 %49, 1
  store i64 %50, ptr %11, align 8
  br label %51

51:                                               ; preds = %55, %46
  %52 = load i64, ptr %10, align 8
  %53 = load i64, ptr %11, align 8
  %54 = icmp slt i64 %52, %53
  br i1 %54, label %55, label %71

55:                                               ; preds = %51
  %56 = load i64, ptr %10, align 8
  %57 = getelementptr inbounds [65 x i8], ptr %7, i64 0, i64 %56
  %58 = load i8, ptr %57, align 1
  store i8 %58, ptr %12, align 1
  %59 = load i64, ptr %11, align 8
  %60 = getelementptr inbounds [65 x i8], ptr %7, i64 0, i64 %59
  %61 = load i8, ptr %60, align 1
  %62 = load i64, ptr %10, align 8
  %63 = getelementptr inbounds [65 x i8], ptr %7, i64 0, i64 %62
  store i8 %61, ptr %63, align 1
  %64 = load i8, ptr %12, align 1
  %65 = load i64, ptr %11, align 8
  %66 = getelementptr inbounds [65 x i8], ptr %7, i64 0, i64 %65
  store i8 %64, ptr %66, align 1
  %67 = load i64, ptr %11, align 8
  %68 = add nsw i64 %67, -1
  store i64 %68, ptr %11, align 8
  %69 = load i64, ptr %10, align 8
  %70 = add nsw i64 %69, 1
  store i64 %70, ptr %10, align 8
  br label %51, !llvm.loop !22

71:                                               ; preds = %51
  %72 = load i64, ptr %4, align 8
  %73 = getelementptr inbounds [65 x i8], ptr %7, i64 0, i64 0
  call void @__print_fd(i64 noundef %72, ptr noundef %73) #5
  br label %74

74:                                               ; preds = %71, %15
  ret void
}

; Function Attrs: noinline nounwind optnone
define dso_local void @__print_float(i64 noundef %0, float noundef %1) #3 {
  %3 = alloca i64, align 8
  %4 = alloca float, align 4
  %5 = alloca i32, align 4
  %6 = alloca float, align 4
  store i64 %0, ptr %3, align 8
  store float %1, ptr %4, align 4
  %7 = load float, ptr %4, align 4
  %8 = fptosi float %7 to i32
  store i32 %8, ptr %5, align 4
  %9 = load float, ptr %4, align 4
  %10 = load i32, ptr %5, align 4
  %11 = sitofp i32 %10 to float
  %12 = fsub float %9, %11
  %13 = fmul float %12, 1.000000e+06
  store float %13, ptr %6, align 4
  %14 = load float, ptr %6, align 4
  %15 = fcmp olt float %14, 0.000000e+00
  br i1 %15, label %16, label %19

16:                                               ; preds = %2
  %17 = load float, ptr %6, align 4
  %18 = fneg float %17
  store float %18, ptr %6, align 4
  br label %19

19:                                               ; preds = %16, %2
  %20 = load i64, ptr %3, align 8
  %21 = load i32, ptr %5, align 4
  %22 = sext i32 %21 to i64
  call void @__print_si(i64 noundef %20, i64 noundef %22, i64 noundef 10) #5
  %23 = load i64, ptr %3, align 8
  call void @__print_fd(i64 noundef %23, ptr noundef @.str.3) #5
  %24 = load i64, ptr %3, align 8
  %25 = load float, ptr %6, align 4
  %26 = fptosi float %25 to i64
  call void @__print_si(i64 noundef %24, i64 noundef %26, i64 noundef 10) #5
  ret void
}

; Function Attrs: noinline nounwind optnone
define dso_local void @__print_double(i64 noundef %0, double noundef %1) #3 {
  %3 = alloca i64, align 8
  %4 = alloca double, align 8
  %5 = alloca i64, align 8
  %6 = alloca double, align 8
  store i64 %0, ptr %3, align 8
  store double %1, ptr %4, align 8
  %7 = load double, ptr %4, align 8
  %8 = fptosi double %7 to i64
  store i64 %8, ptr %5, align 8
  %9 = load double, ptr %4, align 8
  %10 = load i64, ptr %5, align 8
  %11 = sitofp i64 %10 to double
  %12 = fsub double %9, %11
  %13 = fmul double %12, 1.000000e+06
  store double %13, ptr %6, align 8
  %14 = load double, ptr %6, align 8
  %15 = fcmp olt double %14, 0.000000e+00
  br i1 %15, label %16, label %19

16:                                               ; preds = %2
  %17 = load double, ptr %6, align 8
  %18 = fneg double %17
  store double %18, ptr %6, align 8
  br label %19

19:                                               ; preds = %16, %2
  %20 = load i64, ptr %3, align 8
  %21 = load i64, ptr %5, align 8
  call void @__print_si(i64 noundef %20, i64 noundef %21, i64 noundef 10) #5
  %22 = load i64, ptr %3, align 8
  call void @__print_fd(i64 noundef %22, ptr noundef @.str.3) #5
  %23 = load i64, ptr %3, align 8
  %24 = load double, ptr %6, align 8
  %25 = fptosi double %24 to i64
  call void @__print_si(i64 noundef %23, i64 noundef %25, i64 noundef 10) #5
  ret void
}

; Function Attrs: noinline nounwind optnone
define dso_local void @__print_ptr(i64 noundef %0, ptr noundef %1) #3 {
  %3 = alloca i64, align 8
  %4 = alloca ptr, align 8
  store i64 %0, ptr %3, align 8
  store ptr %1, ptr %4, align 8
  %5 = load i64, ptr %3, align 8
  call void @__print_fd(i64 noundef %5, ptr noundef @.str.4) #5
  %6 = load i64, ptr %3, align 8
  %7 = load ptr, ptr %4, align 8
  %8 = ptrtoint ptr %7 to i64
  call void @__print_si(i64 noundef %6, i64 noundef %8, i64 noundef 10) #5
  ret void
}

attributes #0 = { noinline noreturn nounwind optnone "frame-pointer"="all" "min-legal-vector-width"="0" "no-builtins" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { naked noinline nounwind optnone "frame-pointer"="all" "min-legal-vector-width"="0" "no-builtins" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { naked noinline noreturn nounwind optnone "frame-pointer"="all" "min-legal-vector-width"="0" "no-builtins" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { noinline nounwind optnone "frame-pointer"="all" "min-legal-vector-width"="0" "no-builtins" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { nounwind }
attributes #5 = { nobuiltin "no-builtins" }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
!4 = !{!"clang version 20.1.8"}
!5 = !{i64 155, i64 183, i64 211}
!6 = !{i64 390, i64 421, i64 453, i64 486, i64 508, i64 538, i64 566}
!7 = !{i64 703, i64 732, i64 759, i64 786}
!8 = !{i64 935, i64 963, i64 990, i64 1017, i64 1044, i64 1064}
!9 = distinct !{!9, !10}
!10 = !{!"llvm.loop.mustprogress"}
!11 = distinct !{!11, !10}
!12 = distinct !{!12, !10}
!13 = !{i64 1745, i64 1773, i64 1800, i64 1827, i64 1854}
!14 = !{i64 2033, i64 2061, i64 2088, i64 2115, i64 2142}
!15 = !{i64 2333, i64 2361, i64 2388, i64 2415, i64 2442}
!16 = !{i64 2619, i64 2647, i64 2674, i64 2701, i64 2728}
!17 = !{i64 2935, i64 2963, i64 2990, i64 3017, i64 3044}
!18 = !{i64 3249, i64 3277, i64 3304, i64 3331, i64 3358}
!19 = distinct !{!19, !10}
!20 = distinct !{!20, !10}
!21 = distinct !{!21, !10}
!22 = distinct !{!22, !10}
