package com.scut.ui

import android.os.Bundle
import androidx.fragment.app.Fragment
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.RadioGroup
import android.widget.TimePicker
import com.scut.MainActivity
import com.scut.R
import com.scut.databinding.FragmentPsqiBinding

/**
 * A simple [Fragment] subclass.
 * For PSQI self-test
 * create an instance of this fragment.
 */
class PsqiFragment : Fragment() {

    private lateinit var mBinding: FragmentPsqiBinding
    private lateinit var mQuick7to15: Array<RadioGroup>

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?,
                              savedInstanceState: Bundle?): View {
        this.mBinding = FragmentPsqiBinding.inflate(inflater, container, false)
        // 初始化快速数组
        this.mQuick7to15 = arrayOf(this.mBinding.sel7, this.mBinding.sel8, this.mBinding.sel9,
                this.mBinding.sel10, this.mBinding.sel11, this.mBinding.sel12, this.mBinding.sel13,
                this.mBinding.sel14, this.mBinding.sel15)
        // 绑定返回按钮
        this.mBinding.goBack.setOnClickListener { (activity as MainActivity).popFragment() }
        this.partsInit()
        // 刷新全部积分
        this.scoreUpdate()
        // 绑定详情开关动作
        this.mBinding.showDetail.setOnCheckedChangeListener { _, v -> visibilityUpdate(v) }
        // 刷新详情页显示
        this.visibilityUpdate(false)
        return this.mBinding.root
    }

    // 各部分的初始化动作
    private fun partsInit () {
        // 公用的选择刷新器
        val radioListener = RadioGroup.OnCheckedChangeListener { _, _ -> scoreUpdate() }
        // 1 的绑定
        this.mBinding.sel1.setOnCheckedChangeListener(radioListener)
        // 2-3 的绑定
        this.mBinding.question2HourPicker.maxValue = 23
        this.mBinding.question2HourPicker.setOnValueChangedListener { _, _, _ -> scoreUpdate() }
        this.mBinding.question2MinutePicker.maxValue = 59
        this.mBinding.question2MinutePicker.setOnValueChangedListener { _, _, _ -> scoreUpdate() }
        this.mBinding.sel3.setOnCheckedChangeListener(radioListener)
        // 将题6的时长选择器改为 24 小时模式并置为 8.00
        this.mBinding.question6DurationPicker.setIs24HourView(true)
        this.mBinding.question6DurationPicker.hour = 8
        this.mBinding.question6DurationPicker.minute = 0
        // 4、5 也给默认值
        this.mBinding.question4AsleepPicker.hour = 22
        this.mBinding.question4AsleepPicker.minute = 0
        this.mBinding.question5GetUpPicker.hour = 6
        this.mBinding.question5GetUpPicker.minute = 0
        // 题 4、5的选择器更改同时也绑定到更新题6
        val q45Listener = TimePicker.OnTimeChangedListener { _, _, _ ->
            // 计算是否超出范围，是的时候才更新
            val asleepTick = mBinding.question4AsleepPicker.hour * 60 +
                    mBinding.question4AsleepPicker.minute
            val getUpTick = mBinding.question5GetUpPicker.hour * 60 +
                    mBinding.question5GetUpPicker.minute
            // 床上时间
            val onBedTime = when (getUpTick < asleepTick) {
                true -> getUpTick + 24 * 60 - asleepTick
                false -> getUpTick - asleepTick
            }
            // 睡眠时间
            mBinding.question6DurationPicker.hour = onBedTime / 60
            mBinding.question6DurationPicker.minute = onBedTime % 60
            // 总更新
            scoreUpdate()
        }
        this.mBinding.question4AsleepPicker.setOnTimeChangedListener (q45Listener)
        this.mBinding.question5GetUpPicker.setOnTimeChangedListener (q45Listener)
        // 绑定 6 的回调
        this.mBinding.question6DurationPicker.setOnTimeChangedListener { _, _, _ -> scoreUpdate() }
        // 7-5 的绑定
        for (i in this.mQuick7to15) {
            i.setOnCheckedChangeListener(radioListener)
        }
        // 16 - 18 的绑定
        this.mBinding.sel16.setOnCheckedChangeListener(radioListener)
        this.mBinding.sel17.setOnCheckedChangeListener(radioListener)
        this.mBinding.sel18.setOnCheckedChangeListener(radioListener)
    }
    /**
     * 将四选一的题目的序号合并进索引号，未选中为 onFailed，其余的根据id依次为 0-3
     */
    private fun radioGroupCombine (rg: RadioGroup, onFailed: Int = 0): Int {
        return when (rg.checkedRadioButtonId) {
            R.id.sel1sub1,
            R.id.sel3sub1,
            R.id.sel7sub1, R.id.sel8sub1, R.id.sel9sub1,
            R.id.sel10sub1, R.id.sel11sub1, R.id.sel12sub1,
            R.id.sel13sub1, R.id.sel14sub1, R.id.sel15sub1,
            R.id.sel16sub1,
            R.id.sel17sub1, R.id.sel18sub1 -> 0
            
            R.id.sel1sub2,
            R.id.sel3sub2,
            R.id.sel7sub2, R.id.sel8sub2, R.id.sel9sub2,
            R.id.sel10sub2, R.id.sel11sub2, R.id.sel12sub2,
            R.id.sel13sub2, R.id.sel14sub2, R.id.sel15sub2,
            R.id.sel16sub2,
            R.id.sel17sub2, R.id.sel18sub2 -> 1

            R.id.sel1sub3,
            R.id.sel3sub3,
            R.id.sel7sub3, R.id.sel8sub3, R.id.sel9sub3,
            R.id.sel10sub3, R.id.sel11sub3, R.id.sel12sub3,
            R.id.sel13sub3, R.id.sel14sub3, R.id.sel15sub3,
            R.id.sel16sub3,
            R.id.sel17sub3, R.id.sel18sub3 -> 2

            R.id.sel1sub4,
            R.id.sel3sub4,
            R.id.sel7sub4, R.id.sel8sub4, R.id.sel9sub4,
            R.id.sel10sub4, R.id.sel11sub4, R.id.sel12sub4,
            R.id.sel13sub4, R.id.sel14sub4, R.id.sel15sub4,
            R.id.sel16sub4,
            R.id.sel17sub4, R.id.sel18sub4 -> 3

            else -> onFailed
        }
    }

    // 新 -> 原问卷的映射
    // 1->13   2->2    3->5    4->1    5 -> 3    6->4
    // 7-15 -> 6-14    16-18 不变

    private fun calcPartA(): Int {
        val ret = this.radioGroupCombine(mBinding.sel1, 0)
        this.mBinding.PartAPoint.text = ret.toString()
        return ret
    }

    private fun calcPartB(): Int {
        var q2 = this.mBinding.question2HourPicker.value * 60 +
                this.mBinding.question2MinutePicker.value
        q2 = when (q2) {
            in 0 .. 15 -> 0
            in 16 .. 30 -> 1
            in 31 .. 60 -> 2
            else -> 3
        }
        val q23 = q2 + this.radioGroupCombine(this.mBinding.sel3)
        this.mBinding.PartBSum.text = q23.toString()
        val ret = when (q23) {
            0 -> 0
            in 1 .. 2 -> 1
            in 3 .. 4 ->2
            else -> 3
        }
        this.mBinding.PartBPoint.text = ret.toString()
        return ret
    }

    private fun calcPartC(): Int {
        // 此处合并了原问卷的 睡眠时间 与 睡眠效率 两部分
        val asleepTick = this.mBinding.question4AsleepPicker.hour * 60 +
                this.mBinding.question4AsleepPicker.minute
        val getUpTick = this.mBinding.question5GetUpPicker.hour * 60 +
                this.mBinding.question5GetUpPicker.minute
        // 床上时间
        val onBedTime = when (getUpTick < asleepTick) {
            true -> getUpTick + 24 * 60 - asleepTick
            false -> getUpTick - asleepTick
        }
        // 睡眠时间
        val sleepTime = this.mBinding.question6DurationPicker.hour * 60 +
                this.mBinding.question6DurationPicker.minute
        // 睡眠时间得分
        val scoreTime: Int = when (sleepTime) {
            in 0 until (5 * 60) -> 3         // 不到 5 小时
            in (5 * 60) until (6 * 60) -> 2  // 5-6 小时
            in (6 * 60) until (7 * 60) -> 1  // 6-7 小时
            else -> 0                               // 7小时以上
        }
        // 睡眠效率得分
        var scoreEfficient = 0
        if (onBedTime != 0) {
             scoreEfficient = when (sleepTime * 100 / onBedTime) {
                in 0 until 65 -> 3
                in 65..74 -> 2
                in 75..84 -> 1
                else -> 0
            }
        }
        this.mBinding.PartCTimeScore.text = scoreTime.toString()
        this.mBinding.PartCEfficiencyScore.text = scoreEfficient.toString()
        return scoreTime + scoreEfficient
    }

    private fun calcPartD(): Int {
        var total = 0
        for (i in this.mQuick7to15) {
            total += this.radioGroupCombine(i, 0)
        }
        this.mBinding.PartDSum.text = total.toString()
        val ret = when (total) {
            0 -> 0
            in 1 .. 9 -> 1
            in 10 .. 18 -> 2
            else -> 3
        }
        this.mBinding.PartDPoint.text = ret.toString()
        return ret
    }

    private fun calcPartE(): Int {
        val ret = this.radioGroupCombine(this.mBinding.sel16, 0)
        this.mBinding.PartEPoint.text = ret.toString()
        return ret
    }

    private fun calcPartF(): Int {
        val s: Int = this.radioGroupCombine(this.mBinding.sel17, 0) +
                this.radioGroupCombine(this.mBinding.sel18, 0)
        this.mBinding.PartFSum.text = s.toString()
        val ret = when (s) {
            0 -> 0
            1, 2 -> 1
            3, 4 -> 2
            else -> 3
        }
        this.mBinding.PartFPoint.text = ret.toString()
        return ret
    }

    private fun scoreUpdate() {
        // 计算各部分得分
        var total:Int = this.calcPartA()
        total += this.calcPartB()
        total += this.calcPartC()
        total += this.calcPartD()
        total += this.calcPartE()
        total += this.calcPartF()
        // 更新总分
        this.mBinding.sumPoint.text = total.toString()
        // 更新诊断结果
        this.mBinding.result.text = getText(when (total) {
            in 0 .. 5 -> R.string.psqi_test_result_1
            in 6 .. 10 -> R.string.psqi_test_result_2
            in 11 .. 15 -> R.string.psqi_test_result_3
            else -> R.string.psqi_test_result_4
        })
    }

    private fun getCommonText(index: Int, hasDetail: Boolean): CharSequence {
        return getText(when (hasDetail) {
            true -> when (index) {
                    1 ->    R.string.psqi_test_Common_Detail1
                    2 ->    R.string.psqi_test_Common_Detail2
                    3 ->    R.string.psqi_test_Common_Detail3
                    else -> R.string.psqi_test_Common_Detail4
                }
            false -> when (index) {
                    1 ->    R.string.psqi_test_Common_Frequence_1
                    2 ->    R.string.psqi_test_Common_Frequence_2
                    3 ->    R.string.psqi_test_Common_Frequence_3
                    else -> R.string.psqi_test_Common_Frequence_4
                }
        })
    }

    private fun visibilityUpdateA(visibility: Boolean) {
        if (visibility) {
            this.mBinding.partADetail.visibility =  View.VISIBLE
            this.mBinding.sel1sub1.text = getText(R.string.psqi_test_partA_3_detail1)
            this.mBinding.sel1sub2.text = getText(R.string.psqi_test_partA_3_detail2)
            this.mBinding.sel1sub3.text = getText(R.string.psqi_test_partA_3_detail3)
            this.mBinding.sel1sub4.text = getText(R.string.psqi_test_partA_3_detail4)
        }
        else {
            this.mBinding.partADetail.visibility =  View.GONE
            this.mBinding.sel1sub1.text = getText(R.string.psqi_test_partA_3_choice1)
            this.mBinding.sel1sub2.text = getText(R.string.psqi_test_partA_3_choice2)
            this.mBinding.sel1sub3.text = getText(R.string.psqi_test_partA_3_choice3)
            this.mBinding.sel1sub4.text = getText(R.string.psqi_test_partA_3_choice4)
        }
    }

    private fun visibilityUpdateB(visibility: Boolean) {
        val vi = when (visibility) {
            true -> View.VISIBLE
            false -> View.GONE
        }
        // 每题细分显示
        this.mBinding.partBQuestion2Detail.visibility = vi
        this.mBinding.partBQuestion3Detail.visibility = vi
        this.mBinding.sel3sub1.text = this.getCommonText(1, visibility)
        this.mBinding.sel3sub2.text = this.getCommonText(2, visibility)
        this.mBinding.sel3sub3.text = this.getCommonText(3, visibility)
        this.mBinding.sel3sub4.text = this.getCommonText(4, visibility)
        // 总分数显示
        this.mBinding.partBDetail.visibility = vi
    }

    private fun visibilityUpdateC(visibility: Boolean) {
        // 总分数显示，第 6 题细分也在里面
        this.mBinding.partCDetail.visibility = when (visibility) {
            true -> View.VISIBLE
            false -> View.GONE
        }
    }

    private fun visibilityUpdateD(visibility: Boolean) {
        this.mBinding.partDDetail.visibility = when (visibility) {
            true -> View.VISIBLE
            false -> View.GONE
        }
        val sub1 = this.getCommonText(1, visibility)
        val sub2 = this.getCommonText(2, visibility)
        val sub3 = this.getCommonText(3, visibility)
        val sub4 = this.getCommonText(4, visibility)

        this.mBinding.sel7sub1.text = sub1
        this.mBinding.sel8sub1.text = sub1
        this.mBinding.sel9sub1.text = sub1
        this.mBinding.sel10sub1.text = sub1
        this.mBinding.sel11sub1.text = sub1
        this.mBinding.sel12sub1.text = sub1
        this.mBinding.sel13sub1.text = sub1
        this.mBinding.sel14sub1.text = sub1
        this.mBinding.sel15sub1.text = sub1

        this.mBinding.sel7sub2.text = sub2
        this.mBinding.sel8sub2.text = sub2
        this.mBinding.sel9sub2.text = sub2
        this.mBinding.sel10sub2.text = sub2
        this.mBinding.sel11sub2.text = sub2
        this.mBinding.sel12sub2.text = sub2
        this.mBinding.sel13sub2.text = sub2
        this.mBinding.sel14sub2.text = sub2
        this.mBinding.sel15sub2.text = sub2

        this.mBinding.sel7sub3.text = sub3
        this.mBinding.sel8sub3.text = sub3
        this.mBinding.sel9sub3.text = sub3
        this.mBinding.sel10sub3.text = sub3
        this.mBinding.sel11sub3.text = sub3
        this.mBinding.sel12sub3.text = sub3
        this.mBinding.sel13sub3.text = sub3
        this.mBinding.sel14sub3.text = sub3
        this.mBinding.sel15sub3.text = sub3

        this.mBinding.sel7sub4.text = sub4
        this.mBinding.sel8sub4.text = sub4
        this.mBinding.sel9sub4.text = sub4
        this.mBinding.sel10sub4.text = sub4
        this.mBinding.sel11sub4.text = sub4
        this.mBinding.sel12sub4.text = sub4
        this.mBinding.sel13sub4.text = sub4
        this.mBinding.sel14sub4.text = sub4
        this.mBinding.sel15sub4.text = sub4

    }

    private fun visibilityUpdateE(visibility: Boolean) {
        this.mBinding.partEDetail.visibility = when (visibility) {
            true -> View.VISIBLE
            false -> View.GONE
        }
        this.mBinding.sel16sub1.text = this.getCommonText(1, visibility)
        this.mBinding.sel16sub2.text = this.getCommonText(2, visibility)
        this.mBinding.sel16sub3.text = this.getCommonText(3, visibility)
        this.mBinding.sel16sub4.text = this.getCommonText(4, visibility)
    }

    private fun visibilityUpdateF(visibility: Boolean) {
        if (visibility) {
            this.mBinding.partFDetail.visibility =  View.VISIBLE
            this.mBinding.sel17sub1.text = getText(R.string.psqi_test_partF_2_question17_detail1)
            this.mBinding.sel17sub2.text = getText(R.string.psqi_test_partF_2_question17_detail2)
            this.mBinding.sel17sub3.text = getText(R.string.psqi_test_partF_2_question17_detail3)
            this.mBinding.sel17sub4.text = getText(R.string.psqi_test_partF_2_question17_detail4)

            this.mBinding.sel18sub1.text = getText(R.string.psqi_test_partF_3_question18_detail1)
            this.mBinding.sel18sub2.text = getText(R.string.psqi_test_partF_3_question18_detail2)
            this.mBinding.sel18sub3.text = getText(R.string.psqi_test_partF_3_question18_detail3)
            this.mBinding.sel18sub4.text = getText(R.string.psqi_test_partF_3_question18_detail4)
        }
        else {
            this.mBinding.partFDetail.visibility =  View.GONE
            this.mBinding.sel17sub1.text = getText(R.string.psqi_test_partF_2_question17_choice1)
            this.mBinding.sel17sub2.text = getText(R.string.psqi_test_partF_2_question17_choice2)
            this.mBinding.sel17sub3.text = getText(R.string.psqi_test_partF_2_question17_choice3)
            this.mBinding.sel17sub4.text = getText(R.string.psqi_test_partF_2_question17_choice4)

            this.mBinding.sel18sub1.text = getText(R.string.psqi_test_partF_3_question18_choice1)
            this.mBinding.sel18sub2.text = getText(R.string.psqi_test_partF_3_question18_choice2)
            this.mBinding.sel18sub3.text = getText(R.string.psqi_test_partF_3_question18_choice3)
            this.mBinding.sel18sub4.text = getText(R.string.psqi_test_partF_3_question18_choice4)
        }
    }

    private fun visibilityUpdate(visibility: Boolean = false) {
        this.visibilityUpdateA(visibility)
        this.visibilityUpdateB(visibility)
        this.visibilityUpdateC(visibility)
        this.visibilityUpdateD(visibility)
        this.visibilityUpdateE(visibility)
        this.visibilityUpdateF(visibility)
    }
}