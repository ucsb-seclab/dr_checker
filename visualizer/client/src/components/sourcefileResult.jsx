
import React from 'react';
import PropTypes from 'prop-types';
import axios from 'axios';
import config from 'react-global-configuration';
import Grid from 'material-ui/Grid';
import SectionTitle from './sectionTitle.jsx';
import SourcecodeSection from './sourcecodeSection.jsx';
import SummarySection from './summarySection.jsx';
import FullScreenSourceCode from './fullScreenSourcecodeDialog.jsx';


/**
 * This class represent the body of the resultItem.
 * It encapsulatesvarious subcomponents responsble to show for example
 * the highlighted source code, the summary of the analysis relative to this
 * this function, etc...
 */
class SourcefileResult extends React.PureComponent {
  constructor() {
    super();
    this.state = {
      // warnings selected by the users that needs to be highlighted
      selectedWarnings: [],
      // line numbers to highlight
      highlightedLines: [],
      // toggle fullscreen visualization for the sourcecode
      showFullScreen: false,
      sourcecode: 'Loading...',
      alreadyFetchedSourcecode: false,
      legendColors: [],
    };
  }

  /**
   * Fetch the sourcecode for the file separately when the component is mounted
   */
  componentDidMount = () => {
    if (!this.state.alreadyFetchedSourcecode) {
      this.fetchSourcecode();
    }
  }

  /**
   * request the sourcecode 
   */
  fetchSourcecode = () => {
    axios.get(`${config.get('endpoint')}/sourcecode/${this.props.filename.replace(/\//g, '*')}`).then((response) => {
      const parsedData = response.data;
      // we want a random highlight color for each warning
      // POC : create an array of random color
      //       WARNING: everytime the component is refreshed the colors will change randomly
      const colors = this.createLegendColors();
      this.setState({
        alreadyFetchedSourcecode: true,
        sourcecode: parsedData,
        legendColors: colors,
      });
    });
  }

  /**
   * Genearate an array of random colors
   */
  createLegendColors = () => {
    const legendColors = [];
    for (let i = 0; i < this.props.warnings.length; i += 1) {
      legendColors.push(`rgb(${Math.floor(Math.random() * 256)},${Math.floor(Math.random() * 256)},${Math.floor(Math.random() * 256)})`);
    }
    return legendColors;
  }

  /**
   * Extacts which warnings nbeeds to be hioghlighted and their correscponfding line
   */
  handleWarningsSelection = (selectedIds) => {
    const toHighlight = selectedIds.map(v => ({
      lineNo: this.props.warnings[v].warn_data.at_line,
      color: this.state.legendColors[v],
    }));
    this.setState({
      selectedWarnings: selectedIds,
      highlightedLines: toHighlight,
    });
  }

  render() {
    return (
      <div>
        <Grid container>
          <Grid item xs={12}>
            <SectionTitle title={this.props.filename} />
          </Grid>
        </Grid>
        <Grid item xs={12} >
          <SummarySection
            data={this.props.warnings}
            selected={this.state.selectedWarnings}
            handleSelection={this.handleWarningsSelection}
            legendColors={this.state.legendColors}
          />
        </Grid>
        <FullScreenSourceCode title={this.props.filename}>
          <SourcecodeSection
            highlightedLines={this.state.highlightedLines}
            sourcecode={this.state.sourcecode}
          />
        </FullScreenSourceCode>
        <SourcecodeSection
          highlightedLines={this.state.highlightedLines}
          sourcecode={this.state.sourcecode}
          overflowHidden
        />
      </div>
    );
  }
}

SourcefileResult.propTypes = {
  // name of the file related to the result
  filename: PropTypes.string.isRequired,
  // list of warnings for the related sourcefile
  warnings: PropTypes.arrayOf(PropTypes.object).isRequired,
};

export default SourcefileResult;
