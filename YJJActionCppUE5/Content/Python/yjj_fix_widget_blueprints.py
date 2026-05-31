"""
YJJActionCppUE5 — Widget Blueprint 일괄 Reparent + stale GUID 정리 + Compile & Save

에디터 Output Log → Python 실행:
  import yjj_fix_widget_blueprints as fix
  fix.run_all()

또는:
  exec(open(r'D:/source/repos/YJJUnrealAction/YJJActionCppUE5/Content/Python/yjj_fix_widget_blueprints.py').read())
  run_all()

사전 조건: Development Editor 풀 리빌드 후 에디터 재시작 (C++ UCUserWidget_* 클래스 등록).
"""

import unreal

MODULE = "/Script/YJJActionCppUE5"

# 자식 → 부모 순서 (리프 먼저, HUD 루트 마지막)
WIDGET_REPAIRS = [
    # --- Player bars / level ---
    ("/Game/Widgets/Player/CWB_Player_Bar", "PlayerBar"),
    ("/Game/Widgets/Player/CWB_Player_Level", "PlayerLevel"),
    ("/Game/Widgets/Player/WB_Player_HpBar", "PlayerBar"),
    ("/Game/Widgets/Player/WB_Player_ManaBar", "PlayerBar"),
    ("/Game/Widgets/Player/WB_Player_StaminaBar", "PlayerBar"),
    ("/Game/Widgets/Player/WB_Player_Level", "PlayerLevel"),
    # --- Enemy / Boss bars ---
    ("/Game/Widgets/Enemy/CWB_Enemy_HpBar_Guage", "EnemyBar"),
    ("/Game/Widgets/Enemy/WB_Enemy_HpBar_Guage", "EnemyBar"),
    ("/Game/Widgets/Boss/WB_Boss_GroggyBar_Guage", "BossGroggyBarGauge"),
    ("/Game/Widgets/Boss/WB_Boss_GroggyBar", "BossGroggyBar"),
    # --- Menus / interaction / targeting ---
    ("/Game/Widgets/Weapons/CWB_EquipMenuButton", "EquipMenuButton"),
    ("/Game/Widgets/Weapons/WB_EquipMenuButton", "EquipMenuButton"),
    ("/Game/Widgets/Weapons/CWB_EquipMenu", "EquipMenu"),
    ("/Game/Widgets/Weapons/WB_EquipMenu", "EquipMenu"),
    ("/Game/Widgets/Weapons/WB_MagicMenu", "MagicMenu"),
    ("/Game/Widgets/Weapons/WB_MagicMenuButton", "EquipMenuButton"),
    ("/Game/Widgets/Interaction/CWB_Interaction", "Interaction"),
    ("/Game/Widgets/Interaction/WB_Interaction", "Interaction"),
    ("/Game/Widgets/Interaction/CWB_Targeting", "Custom"),
    ("/Game/Widgets/Enemy/WB_Targeting", "Targeting"),
    # --- Player info / boss info ---
    ("/Game/Widgets/Player/CWB_Player_Info", "PlayerInfo"),
    ("/Game/Widgets/Player/WB_Player_Info", "PlayerInfo"),
    ("/Game/Widgets/Boss/WB_Boss_Info", "Custom"),
    # --- HUD roots (마지막) ---
    ("/Game/Widgets/WB_HUDUI", "HUD"),
    ("/Game/Widgets/CWB_HUD", "HUD"),
]


def load_cpp_widget_class(class_suffix: str):
    """UCUserWidget_* / CUserWidget_* 둘 다 시도."""
    for name in (f"UCUserWidget_{class_suffix}", f"CUserWidget_{class_suffix}"):
        path = f"{MODULE}.{name}"
        cls = unreal.load_class(None, path)
        if cls:
            return cls, path
    return None, None


def collect_seen_widget_names(widget_bp: unreal.WidgetBlueprint) -> set:
    seen = set()
    widget_tree = widget_bp.get_editor_property("widget_tree")
    if widget_tree:
        all_widgets = widget_tree.get_editor_property("all_widgets")
        if all_widgets:
            for widget in all_widgets:
                if widget:
                    seen.add(str(widget.get_fname()))
    animations = widget_bp.get_editor_property("animations")
    if animations:
        for anim in animations:
            if anim:
                seen.add(str(anim.get_fname()))
    return seen


def prune_stale_widget_variable_guids(widget_bp: unreal.WidgetBlueprint) -> int:
    """ValidateAndFixUpVariableGuids Ensure 전에 디스크에 남은 orphan GUID 제거."""
    seen = collect_seen_widget_names(widget_bp)
    guid_map = widget_bp.get_editor_property("widget_variable_name_to_guid_map")
    if not guid_map:
        return 0

    removed = 0
    stale_keys = [key for key in list(guid_map.keys()) if str(key) not in seen]
    for key in stale_keys:
        del guid_map[key]
        removed += 1

    if removed > 0:
        widget_bp.set_editor_property("widget_variable_name_to_guid_map", guid_map)
        widget_bp.modify()
    return removed


def reparent_widget_blueprint(asset_path: str, class_suffix: str) -> bool:
    widget_bp = unreal.load_asset(asset_path)
    if not widget_bp:
        unreal.log_warning(f"[YJJ WidgetFix] SKIP (not found): {asset_path}")
        return False

    if not isinstance(widget_bp, unreal.WidgetBlueprint):
        unreal.log_warning(f"[YJJ WidgetFix] SKIP (not WidgetBlueprint): {asset_path}")
        return False

    parent_cls, parent_path = load_cpp_widget_class(class_suffix)
    if not parent_cls:
        unreal.log_error(f"[YJJ WidgetFix] FAIL parent class: {class_suffix} for {asset_path}")
        return False

    current_parent = widget_bp.get_editor_property("parent_class")
    current_name = current_parent.get_name() if current_parent else "(none)"
    target_name = parent_cls.get_name()

    if current_name == target_name:
        unreal.log(f"[YJJ WidgetFix] OK already: {asset_path} -> {target_name}")
    else:
        unreal.log(f"[YJJ WidgetFix] Reparent: {asset_path} {current_name} -> {target_name} ({parent_path})")
        unreal.BlueprintEditorLibrary.reparent_blueprint(widget_bp, parent_cls)

    pruned = prune_stale_widget_variable_guids(widget_bp)
    if pruned > 0:
        unreal.log(f"[YJJ WidgetFix] Pruned {pruned} stale GUID(s): {asset_path}")

    compile_ok = unreal.BlueprintEditorLibrary.compile_blueprint(widget_bp)
    if not compile_ok:
        unreal.log_error(f"[YJJ WidgetFix] COMPILE FAILED: {asset_path}")
        return False

    saved = unreal.EditorAssetLibrary.save_loaded_asset(widget_bp, only_if_is_dirty=False)
    if not saved:
        unreal.log_error(f"[YJJ WidgetFix] SAVE FAILED: {asset_path}")
        return False

    unreal.log(f"[YJJ WidgetFix] DONE: {asset_path}")
    return True


def verify_parent_classes() -> bool:
    ok = True
    test_suffixes = ["Custom", "HUD", "PlayerBar", "PlayerInfo", "BossGroggyBar", "BossGroggyBarGauge"]
    for suffix in test_suffixes:
        cls, path = load_cpp_widget_class(suffix)
        if cls:
            unreal.log(f"[YJJ WidgetFix] parent OK: {path}")
        else:
            unreal.log_error(f"[YJJ WidgetFix] parent MISSING: {suffix} — 리빌드 후 재시도")
            ok = False
    return ok


def run_all():
    unreal.log("[YJJ WidgetFix] === 시작 ===")
    if not verify_parent_classes():
        unreal.log_error("[YJJ WidgetFix] C++ 부모 클래스 로드 실패 — Development Editor 리빌드 필요")
        return

    success = 0
    failed = 0
    for asset_path, class_suffix in WIDGET_REPAIRS:
        if reparent_widget_blueprint(asset_path, class_suffix):
            success += 1
        else:
            failed += 1

    unreal.log(f"[YJJ WidgetFix] === 완료: success={success} failed={failed} ===")
    unreal.log("[YJJ WidgetFix] 에디터 완전 종료 후 재시작 → CWB_HUD / WB_Boss_Info 열림 확인")


if __name__ == "__main__":
    run_all()
