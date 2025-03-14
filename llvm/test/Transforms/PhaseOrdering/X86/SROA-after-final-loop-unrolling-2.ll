; NOTE: Assertions have been autogenerated by utils/update_test_checks.py
; RUN: opt < %s -O3 -instcombine-disable-v810-fix -S                   | FileCheck %s
; RUN: opt < %s -passes="default<O3>" -instcombine-disable-v810-fix -S | FileCheck %s
; LoopFullUnrollPass will not preform non-full unrolling,
; yet until we unroll at least 2x,
; the loads/stores into `%i3 = alloca %t0, align 4`
; happen at non-constant indexes.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%t0 = type { [2 x i32] }

$_ZNSt5arrayIiLm2EEixEm = comdat any

$_ZNSt14__array_traitsIiLm2EE6_S_refERA2_Kim = comdat any

@global = private unnamed_addr constant %t0 { [2 x i32] [i32 24, i32 42] }, align 4

; Function Attrs: mustprogress nounwind uwtable
define dso_local void @foo(i32 noundef %arg, ptr noundef nonnull align 4 dereferenceable(8) %arg1) #0 {
; CHECK-LABEL: @foo(
; CHECK-NEXT:  bb:
; CHECK-NEXT:    [[I9:%.*]] = sdiv i32 [[ARG:%.*]], 128
; CHECK-NEXT:    [[I10:%.*]] = shl nsw i32 [[I9]], 7
; CHECK-NEXT:    [[ARG_OFF:%.*]] = add i32 [[ARG]], 127
; CHECK-NEXT:    [[TMP0:%.*]] = icmp ult i32 [[ARG_OFF]], 255
; CHECK-NEXT:    br i1 [[TMP0]], label [[BB12:%.*]], label [[BB13:%.*]]
; CHECK:       bb12.loopexit:
; CHECK-NEXT:    [[I3_SROA_8_0_INSERT_EXT:%.*]] = zext i32 [[I21_3:%.*]] to i64
; CHECK-NEXT:    [[I3_SROA_8_0_INSERT_SHIFT:%.*]] = shl nuw i64 [[I3_SROA_8_0_INSERT_EXT]], 32
; CHECK-NEXT:    [[I3_SROA_0_0_INSERT_EXT:%.*]] = zext i32 [[I21_2:%.*]] to i64
; CHECK-NEXT:    [[I3_SROA_0_0_INSERT_INSERT:%.*]] = or disjoint i64 [[I3_SROA_8_0_INSERT_SHIFT]], [[I3_SROA_0_0_INSERT_EXT]]
; CHECK-NEXT:    br label [[BB12]]
; CHECK:       bb12:
; CHECK-NEXT:    [[TMP1:%.*]] = phi i64 [ [[I3_SROA_0_0_INSERT_INSERT]], [[BB12_LOOPEXIT:%.*]] ], [ 180388626456, [[BB:%.*]] ]
; CHECK-NEXT:    store i64 [[TMP1]], ptr [[ARG1:%.*]], align 4, !tbaa [[TBAA5:![0-9]+]]
; CHECK-NEXT:    ret void
; CHECK:       bb13:
; CHECK-NEXT:    [[I3_SROA_8_0:%.*]] = phi i32 [ [[I21_3]], [[BB13]] ], [ 42, [[BB]] ]
; CHECK-NEXT:    [[I3_SROA_0_0:%.*]] = phi i32 [ [[I21_2]], [[BB13]] ], [ 24, [[BB]] ]
; CHECK-NEXT:    [[I4_05:%.*]] = phi i32 [ [[I24_3:%.*]], [[BB13]] ], [ 0, [[BB]] ]
; CHECK-NEXT:    [[I21:%.*]] = mul nsw i32 [[I3_SROA_0_0]], [[I4_05]]
; CHECK-NEXT:    [[I24:%.*]] = or disjoint i32 [[I4_05]], 1
; CHECK-NEXT:    [[I21_1:%.*]] = mul nsw i32 [[I3_SROA_8_0]], [[I24]]
; CHECK-NEXT:    [[I24_1:%.*]] = or disjoint i32 [[I4_05]], 2
; CHECK-NEXT:    [[I21_2]] = mul nsw i32 [[I21]], [[I24_1]]
; CHECK-NEXT:    [[I24_2:%.*]] = or disjoint i32 [[I4_05]], 3
; CHECK-NEXT:    [[I21_3]] = mul nsw i32 [[I21_1]], [[I24_2]]
; CHECK-NEXT:    [[I24_3]] = add nuw nsw i32 [[I4_05]], 4
; CHECK-NEXT:    [[I11_NOT_3:%.*]] = icmp eq i32 [[I24_3]], [[I10]]
; CHECK-NEXT:    br i1 [[I11_NOT_3]], label [[BB12_LOOPEXIT]], label [[BB13]], !llvm.loop [[LOOP8:![0-9]+]]
;
bb:
  %i = alloca i32, align 4
  %i2 = alloca ptr, align 8
  %i3 = alloca %t0, align 4
  %i4 = alloca i32, align 4
  %i5 = alloca ptr, align 8
  store i32 %arg, ptr %i, align 4, !tbaa !5
  store ptr %arg1, ptr %i2, align 8, !tbaa !9
  call void @llvm.lifetime.start.p0(i64 8, ptr %i3) #3
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %i3, ptr align 4 @global, i64 8, i1 false)
  call void @llvm.lifetime.start.p0(i64 4, ptr %i4) #3
  store i32 0, ptr %i4, align 4, !tbaa !5
  br label %bb6

bb6:                                              ; preds = %bb22, %bb
  %i7 = load i32, ptr %i4, align 4, !tbaa !5
  %i8 = load i32, ptr %i, align 4, !tbaa !5
  %i9 = sdiv i32 %i8, 128
  %i10 = mul nsw i32 128, %i9
  %i11 = icmp ne i32 %i7, %i10
  br i1 %i11, label %bb13, label %bb12

bb12:                                             ; preds = %bb6
  call void @llvm.lifetime.end.p0(i64 4, ptr %i4) #3
  br label %bb25

bb13:                                             ; preds = %bb6
  call void @llvm.lifetime.start.p0(i64 8, ptr %i5) #3
  %i14 = load i32, ptr %i4, align 4, !tbaa !5
  %i15 = srem i32 %i14, 2
  %i16 = sext i32 %i15 to i64
  %i17 = call noundef nonnull align 4 dereferenceable(4) ptr @wibble(ptr noundef nonnull align 4 dereferenceable(8) %i3, i64 noundef %i16) #3
  store ptr %i17, ptr %i5, align 8, !tbaa !9
  %i18 = load i32, ptr %i4, align 4, !tbaa !5
  %i19 = load ptr, ptr %i5, align 8, !tbaa !9
  %i20 = load i32, ptr %i19, align 4, !tbaa !5
  %i21 = mul nsw i32 %i20, %i18
  store i32 %i21, ptr %i19, align 4, !tbaa !5
  call void @llvm.lifetime.end.p0(i64 8, ptr %i5) #3
  br label %bb22

bb22:                                             ; preds = %bb13
  %i23 = load i32, ptr %i4, align 4, !tbaa !5
  %i24 = add nsw i32 %i23, 1
  store i32 %i24, ptr %i4, align 4, !tbaa !5
  br label %bb6, !llvm.loop !11

bb25:                                             ; preds = %bb12
  %i26 = load ptr, ptr %i2, align 8, !tbaa !9
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %i26, ptr align 4 %i3, i64 8, i1 false), !tbaa.struct !13
  call void @llvm.lifetime.end.p0(i64 8, ptr %i3) #3
  ret void
}

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: argmemonly nocallback nofree nounwind willreturn
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #2

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local noundef nonnull align 4 dereferenceable(4) ptr @wibble(ptr noundef nonnull align 4 dereferenceable(8) %arg, i64 noundef %arg1) #0 comdat($_ZNSt5arrayIiLm2EEixEm) align 2 {
bb:
  %i = alloca ptr, align 8
  %i2 = alloca i64, align 8
  store ptr %arg, ptr %i, align 8, !tbaa !9
  store i64 %arg1, ptr %i2, align 8, !tbaa !15
  %i3 = load ptr, ptr %i, align 8
  %i5 = load i64, ptr %i2, align 8, !tbaa !15
  %i6 = call noundef nonnull align 4 dereferenceable(4) ptr @widget(ptr noundef nonnull align 4 dereferenceable(8) %i3, i64 noundef %i5) #3
  ret ptr %i6
}

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: mustprogress nounwind uwtable
define linkonce_odr dso_local noundef nonnull align 4 dereferenceable(4) ptr @widget(ptr noundef nonnull align 4 dereferenceable(8) %arg, i64 noundef %arg1) #0 comdat($_ZNSt14__array_traitsIiLm2EE6_S_refERA2_Kim) align 2 {
bb:
  %i = alloca ptr, align 8
  %i2 = alloca i64, align 8
  store ptr %arg, ptr %i, align 8, !tbaa !9
  store i64 %arg1, ptr %i2, align 8, !tbaa !15
  %i3 = load ptr, ptr %i, align 8, !tbaa !9
  %i4 = load i64, ptr %i2, align 8, !tbaa !15
  %i5 = getelementptr inbounds [2 x i32], ptr %i3, i64 0, i64 %i4
  ret ptr %i5
}

attributes #0 = { mustprogress nounwind uwtable "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { argmemonly nocallback nofree nosync nounwind willreturn }
attributes #2 = { argmemonly nocallback nofree nounwind willreturn }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"Debian clang version 16.0.0-++20221024100939+996267d20ea0-1~exp1~20221024221046.844"}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C++ TBAA"}
!9 = !{!10, !10, i64 0}
!10 = !{!"any pointer", !7, i64 0}
!11 = distinct !{!11, !12}
!12 = !{!"llvm.loop.mustprogress"}
!13 = !{i64 0, i64 8, !14}
!14 = !{!7, !7, i64 0}
!15 = !{!16, !16, i64 0}
!16 = !{!"long", !7, i64 0}
