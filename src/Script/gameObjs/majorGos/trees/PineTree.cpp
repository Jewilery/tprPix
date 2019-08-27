/*
 * ========================= PineTree.cpp ==========================
 *                          -- tpr --
 *                                        CREATE -- 2019.04.05
 *                                        MODIFY -- 
 * ----------------------------------------------------------
 */
#include "Script/gameObjs/majorGos/trees/PineTree.h"

//-------------------- CPP --------------------//
#include <functional>
#include <string>

//-------------------- tpr --------------------//
#include "tprGeneral.h"

//-------------------- Engine --------------------//
#include "tprAssert.h"
#include "GoCreateDyParams_tree.h"
#include "esrc_shader.h" 

//-------------------- Script --------------------//
#include "Script/resource/ssrc.h" 
#include "Script/gameObjs/create_go_oth.h"


using namespace std::placeholders;

#include "tprDebug.h" 


namespace gameObjs{//------------- namespace gameObjs ----------------

namespace PineTree_inn {//-------------- namespace: PineTree_inn ------------------//

    //--- 将所有 pineId 分类，方便分配 ---
    std::vector<size_t> ids_age1   { 0, 1 };
    //---
    std::vector<size_t> ids_age2   { 2, 3 };
    //---
    std::vector<size_t> ids_age3   { 4, 5, 6, 7 };
    //---
    std::vector<size_t> ids_age4   { 7 }; //- 暂无


    //===== funcs =====//
    size_t apply_a_oakId( int _age, double fieldWeight_ );


}//------------------ namespace: PineTree_inn end ------------------//


/* ===========================================================
 *                 init_in_autoMod
 * -----------------------------------------------------------
 */
void PineTree::init_in_autoMod( GameObj &goRef_,
                                const ParamBinary &dyParams_ ){

    //================ dyParamBinary =================//
    tprAssert( dyParams_.type == ParamBinaryType::Tree );
    const GoCreateDyParams_tree *dyParamsPtr = reinterpret_cast<const GoCreateDyParams_tree*>(dyParams_.get_binaryPtr());

    //================ go.pvtBinary =================//
    goRef_.resize_pvtBinary( sizeof(PineTree_PvtBinary) );
    PineTree_PvtBinary *pvtBp = reinterpret_cast<PineTree_PvtBinary*>(goRef_.get_pvtBinaryPtr());

        pvtBp->age = gameObjs::apply_treeAge_by_density( dyParamsPtr->fieldDensity );
        pvtBp->pineId = PineTree_inn::apply_a_oakId( pvtBp->age, dyParamsPtr->fieldWeight );
        //...

    //================ animFrameSet／animFrameIdxHandle/ goMesh =================//
        //-- 制作唯一的 mesh 实例: "root" --
        goRef_.creat_new_goMesh("root", //- gmesh-name
                                "pineTree", 
                                tprGeneral::nameString_combine("", pvtBp->pineId, "_idle"),
                                RenderLayerType::MajorGoes, //- 不设置 固定zOff值
                                &esrc::get_rect_shader(),  // pic shader
                                glm::vec2{ 0.0f, 0.0f }, //- pposoff
                                0.0,  //- off_z
                                true, //- isVisible
                                true //- isCollide
                                );

    //================ bind callback funcs =================//
    //-- 故意将 首参数this 绑定到 保留类实例 dog_a 身上
    goRef_.RenderUpdate = std::bind( &PineTree::OnRenderUpdate,  _1 );   
    goRef_.LogicUpdate  = std::bind( &PineTree::OnLogicUpdate,   _1 );
    
    //-------- actionSwitch ---------//
    goRef_.actionSwitch.bind_func( std::bind( &PineTree::OnActionSwitch, _1, _2 ) );
    goRef_.actionSwitch.signUp( ActionSwitchType::Move_Idle );
                //- 当前树木只有一种动画，就是永久待机...

    //================ go self vals =================//

    //--- 小树，中树 可以被其它go 穿过，成年树不行 ---
    if( pvtBp->age <= 2 ){
        goRef_.set_collision_isBePass( true );
    }else{
        goRef_.set_collision_isBePass( false );
    }
    
    //-- 务必在 mesh:"root" 之后 ---
    goRef_.init_goPos_currentDPos( );
    //...   

    //--- MUST ---
    goRef_.init_check(); 
}


/* ===========================================================
 *                       bind
 * -----------------------------------------------------------
 * -- 在 “工厂”模式中，将本具象go实例，与 一个已经存在的 go实例 绑定。
 * -- 这个 go实例 的类型，应该和 本类一致。
 */
void PineTree::bind( GameObj &goRef_ ){
}


/* ===========================================================
 *                       rebind
 * -----------------------------------------------------------
 * -- 从硬盘读取到 go实例数据后，重bind callback
 * -- 会被 脚本层的一个 巨型分配函数 调用
 */
void PineTree::rebind( GameObj &goRef_ ){
}


/* ===========================================================
 *                      OnRenderUpdate
 * -----------------------------------------------------------
 */
void PineTree::OnRenderUpdate( GameObj &goRef_ ){
    //=====================================//
    //            ptr rebind
    //-------------------------------------//
    PineTree_PvtBinary  *pvtBp = PineTree::rebind_ptr( goRef_ );

    //=====================================//
    //           test: AI
    //-------------------------------------//
    //...

    //=====================================//
    //         更新 位移系统
    //-------------------------------------//
    //goRef_.move.RenderUpdate();
            // 目前来看，永远也不会 移动...


    //=====================================//
    //  将 确认要渲染的 goMeshs，添加到 renderPool         
    //-------------------------------------//
    goRef_.render_all_goMesh();
}


/* ===========================================================
 *                        OnLogicUpdate
 * -----------------------------------------------------------
 */
void PineTree::OnLogicUpdate( GameObj &goRef_ ){
    //=====================================//
    //            ptr rebind
    //-------------------------------------//
    PineTree_PvtBinary  *pvtBp = PineTree::rebind_ptr( goRef_ );
    //=====================================//

    // 什么也没做...
}


/* ===========================================================
 *               OnActionSwitch
 * -----------------------------------------------------------
 * -- 此处用到的 animFrameIdxHdle实例，是每次用到时，临时 生产／改写 的
 * -- 会被 动作状态机 取代...
 */
void PineTree::OnActionSwitch( GameObj &goRef_, ActionSwitchType type_ ){

    cout << "PineTree::OnActionSwitch()"
        << endl;

    //=====================================//
    //            ptr rebind
    //-------------------------------------//
    PineTree_PvtBinary  *pvtBp = PineTree::rebind_ptr( goRef_ );
    //=====================================//

    //-- 获得所有 goMesh 的访问权 --
    //GameObjMesh &rootGoMeshRef = goRef_.goMeshs.at("root");

    //-- 处理不同的 actionSwitch 分支 --
    switch( type_ ){
        case ActionSwitchType::Move_Idle:
            //rootGoMeshRef.bind_animFrameSet( "norman" );
            //rootGoMeshRef.getnc_animFrameIdxHandle().bind_idle( pvtBp->oakId );
                                    
            break;

        default:
            break;
            //-- 并不报错，什么也不做...

    }
}


namespace PineTree_inn {//-------------- namespace: PineTree_inn ------------------//



/* ===========================================================
 *                     apply_a_oakId
 * -----------------------------------------------------------
 */
size_t apply_a_oakId( int _age, double fieldWeight_ ){
    size_t  idx {};
    size_t randV = gameObjs::apply_a_simpleId( fieldWeight_, 79 );

    switch( _age ){
        case 1: idx = randV % ids_age1.size();  return ids_age1.at(idx);
        case 2: idx = randV % ids_age2.size();  return ids_age2.at(idx);
        case 3: idx = randV % ids_age3.size();  return ids_age3.at(idx);
        case 4: idx = randV % ids_age4.size();  return ids_age4.at(idx);
        default:
            assert(0);
            return 0; //- never touch
    }
}



}//------------------ namespace: PineTree_inn end ------------------//
}//------------- namespace gameObjs: end ----------------
